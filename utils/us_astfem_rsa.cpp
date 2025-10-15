//! \file us_astfem_rsa.cpp

#include "us_astfem_rsa.h"
#include "us_astfem_math.h"
#include "us_hardware.h"
#include "us_math2.h"
#include "us_memory.h"
#include "us_stiffbase.h"
#include "us_settings.h"
#include "us_sleep.h"
#include "us_util.h"
#include "us_time_state.h"
#include <unistd.h>
#include <algorithm>
#ifdef Q_OS_WIN         // Include headers so getpid() works on Windows
#include <windows.h>
#include <psapi.h>
#include <process.h>
#endif

#if 0                   // Set to 1 to enable RA timing prints
#define TIMING_RA_INC 500
#define TIMING_RA
#endif

#if 0                   // Set to 1 to enable NI timing prints
#define TIMING_NI 1
#endif

// Constructor for US_Astfem_RSA structure
US_Astfem_RSA::US_Astfem_RSA( US_Model&                model,
                              US_SimulationParameters& params,
                              QObject*                 parent )
   : QObject( parent ), system( model ), simparams( params )
{
   stopFlag        = false ; // Flag used to stop the simulation.
   use_time        = true ;  // Flag used for choosing interpolation option.
                             // True for time based interpolation and false for
                             // omega_square_t based interpolation.
   time_correction = false ;
   simout_flag     = false ; // Flag used for choosing the display grid for scans.
                             // True refers to display on simulation grid and false
                             // refer to display on experimental grid.
   show_movie      = false;  // Flag used to see a movie i.e. movement of scans.
   dbg_level       = 0  ;    // Flag used to choose a debug level.
}

//!< Takes the experimental data i.e. 'exp_data' as input and updates the scans
//!< with concentration values.

//!< We are solving for Lamm equation to get concentration values on the
//!< grid points.

//!< Here we deal with two radial grid systems. One the user chooses
//!< for the output and the other radial grid needed for finite element simulation.
//!< Time grid spacing is different from radial grid spacing and defined in
//!< Ref : Cao W, Demeler B, Modeling Analytical Ultracentrifugation Experiments
//         with an Adaptive Space-Time Finite Element Solution of the Lamm Equation,
//         (2005) Biophys J. 89(3):1589-602.

//!< Concentrations can be obtained on either experimental grid or
//!< finite element simulation grid based on user's choice. "simout_flag" is the
//!< variable used to choose grid, true refers to simulation grid and false
//!< refers to experimental grid.

//!< If user wants solution on experimental grid, the simulated solution is
//!< interpolated from simulation grid onto experimental grid.

//!< Both reacting and non-reacting systems are handled here. calculate_ni takes
//!< care for non-reacting systems and calculate_ra2 takes care for reacting systems.
int US_Astfem_RSA::calculate( US_DataIO::RawData& exp_data )
{
   US_AstfemMath::MfemInitial* vC0 = NULL; // Initial concentration for multiple components
   US_AstfemMath::MfemInitial  CT0;        // Initial  concentration vector
   US_AstfemMath::MfemData     simdata;    // Contains scans on the simulation grid
   double        current_time   = 0.0;     // Initialize time
   double        current_om2t   = 0.0;     // Initialize omega square t
   double        current_speed  = 0.0;     // Initialize rotor speed
   double        duration       = 0.0;     // Initialize duration of the experiment
//   double        delay          = 0.0;     // Initialize delay
//   int           current_assoc  = 0;       // Current association
   int           size_cv        = system.components.size(); // Number of components in the model

   QVector< bool > reactVec( size_cv );    // Reacting vectors
   bool*         reacting       = reactVec.data();  // Used to check reacting or non_reacting case
   double        accel_time     = 0.0;              // Duration to reach at next rotor speed
   double        dr;                                // Increment on the initial radial grid

   QList< int >    accel_times;   // time when acceleration ended
   QList< double > accel_w2ts;    // w2t at end of acceleration
   QList< int >    time_end ;     // time at end of the step
   QList< double > w2t_end;       // w2t  at end of the step
   US_SimulationParameters::SpeedProfile* sp;  // Pointer used to current speed SpeedProfile

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

   // Compute the total concentration for the model
   tot_conc           = 0.0;
   for ( int cc = 0; cc < size_cv; cc++ )
   {
      tot_conc          += system.components[ cc ].signal_concentration;
DbgLv(1)<< "RSA:calc: cc" << cc << "tot_conc" << tot_conc;
   }

   // Find the speed step index for the current dataset
   int cdsx           = 0;                       // Current dataset's step index
   int lsx            = exp_data.scanCount() - 1;          // Last scan index
   double fstime      = exp_data.scanData[   0 ].seconds;  // First scan time
   double lstime      = exp_data.scanData[ lsx ].seconds;  // Last scan time
DbgLv(1)<< "RSA:calc:  lsx fstime lstime" << lsx << fstime << lstime;

   for ( int step = 0; step < simparams.speed_step.count(); step++ )
   {
      if ( fstime >=  simparams.speed_step[ step ].time_first  &&
           lstime <=  simparams.speed_step[ step ].time_last )
      {  // Found step that this dataset fits inside:  break out
         cdsx               = step;
DbgLv(1)<< "RSA:calc:   cdsx" << cdsx << "time_first time_last"
 << simparams.speed_step[ step ].time_first << simparams.speed_step[ step ].time_last;
         break;
      }
   }
//DbgLv(0)<< "RSA:calc ===ST===";
//printsimparams( );
//DbgLv(0)<< "RSA:calc ===EN===";

   // Initialization of different parameters for simulation begins here-
   af_params.simpoints   = simparams.simpoints;     // Number of simulation i.e. radial grid points
   af_params.pathlength  = simparams.cp_pathlen;    // Pathlength
   af_params.dt          = 1.0;                     // Time gap between two time steps
   af_params.time_steps  = simparams.simpoints;     // Number of time steps
   af_params.cdset_speed = simparams.speed_step[ cdsx ].rotorspeed; // Rotor speed of current dataset
DbgLv(1)<< "RSA:calc:    cdset_speed" << af_params.cdset_speed;
   double s0speed        = simparams.speed_step[ 0 ].rotorspeed;
   af_params.omega_s     = sq( s0speed * M_PI / 30.0 ); // omega_square
   af_params.start_om2t  = af_params.omega_s;       // Starting omega square t
   af_params.start_time  = 0.0;                     // Starting time
   af_params.bottom_pos  = simparams.bottom_position;   // Centerpiece bottom
   if ( simparams.bottom > simparams.bottom_position )
   {
      double stretch_val  = stretch( simparams.rotorcoeffs,
                                     (int)af_params.cdset_speed );
      double bottom_clc   = simparams.bottom_position + stretch_val;
      double bott_diff    = simparams.bottom - bottom_clc;
      if ( qAbs( bott_diff ) > 1.e-5 )
      {
         af_params.bottom_pos  = simparams.bottom_position + bott_diff;
      }
   }
#if 1
   adjust_limits( af_params.cdset_speed );          // Does rotor stretch
#endif
#if 0
   af_params.current_meniscus = simparams.meniscus; // Meniscus from simparams structure
   af_params.current_bottom   = simparams.bottom;   // Bottom from simparams structure
#endif
DbgLv(1)<< "RSA:calc: meniscus bottom" << af_params.current_meniscus << af_params.current_bottom;

   // Loads the experimental data exp_data of 'RawData'type
   //   to af_data of 'MfemData' type.
   // af_data is used for getting scans on experimental grid
   load_mfem_data( exp_data, af_data );

   int initial_npts      = af_data.scan[ 0 ].conc.size(); // Size of the concentration vector on radial grid

   // Set up reaction groups--
   initialize_rg();

   // Updates the bottom and meniscus for experimental grid
   af_data.meniscus = af_params.current_meniscus;
   af_data.bottom   = af_params.current_bottom;
   double omeg0     = 0.0;
   double omeg2     = 0.0;
   simparams.sim    = ( exp_data.channel == 'S' );

   // Read in any timestate that exists and set up the internal
   //  simulation speed profile
   if ( simparams.tsobj == NULL  ||
        simparams.sim_speed_prof.count() < 1 )
   {  // Timestate is not properly loaded
DbgLv(1)<<"RSA:calc: timestate does not exist";
#ifdef NO_DB
      QString tmst_fpath = "../" + temp_Id_name() + ".time_state.tmst";

      if ( ! QFile( tmst_fpath ).exists() )
         US_AstfemMath::writetimestate( tmst_fpath, simparams, exp_data );
#else
      QString tmst_fpath = US_Settings::tmpDir() + "/" + temp_Id_name() + ".time_state.tmst";
      US_AstfemMath::writetimestate( tmst_fpath, simparams, exp_data );
#endif
      simparams.simSpeedsFromTimeState( tmst_fpath );
   }
   else
   {  // Timestate object and speed profile exist are properly loaded
DbgLv(1) << "RSA:calc : timestate exists and timestateobject,sscount="
 << simparams.tsobj << simparams.sim_speed_prof.count();
   }

   int nstep    = simparams.speed_step.size();     // Number of speed steps
   int nspstep  = simparams.sim_speed_prof.size(); // Number of speed profiles
DbgLv(1) << "RSA:calc: ss size" << nstep << "ssp size" << nspstep;

   for ( int istep = 0; istep < nspstep; istep++ )
   {  // Fill time,omega2t work vectors for each step
      time_end    << simparams.sim_speed_prof[ istep ].time_e_step;
      w2t_end     << simparams.sim_speed_prof[ istep ].w2t_e_step;
      accel_times << simparams.sim_speed_prof[ istep ].time_e_accel;
      accel_w2ts  << simparams.sim_speed_prof[ istep ].w2t_e_accel;

//*DEBUG*
if(dbg_level>0) {
 DbgLv(1) << "RSA:calc: readings:from tmst file: istep" << istep
  << "duration" << simparams.sim_speed_prof[istep].duration
  << "time_ea" << accel_times[istep] << "time_e" << time_end[istep]
  << "w2t_ea" << accel_w2ts[istep] << "w2t_e" << w2t_end[istep];
 for ( int jt=0; jt<simparams.sim_speed_prof[istep].duration; jt++ ) {
  if(jt<4 || (jt+4)>simparams.sim_speed_prof[istep].duration)
   DbgLv(1) << "RSA:calc::  from_tmst:  rpm" << simparams.sim_speed_prof[istep].rpm_timestate[jt]
    << "time" << simparams.sim_speed_prof[istep].time_b_accel+jt
    << "w2t" << simparams.sim_speed_prof[istep].w2t_timestate[jt];
}
}
//*DEBUG*
   }

   if ( nstep > 0  &&  nstep < nspstep )
   {  // There are missing speed steps
      US_SimulationParameters::SpeedProfile sp0 = simparams.speed_step[ 0 ];
      QVector< US_SimulationParameters::SpeedProfile > ss_old = simparams.speed_step;
      QList< int > speeds;
DbgLv(1) << "RSA:calc: SSS:  nstep nspstep" << nstep << nspstep << "cdsx" << cdsx;

      for ( int ss = 0; ss < nstep; ss++ )
      {
DbgLv(1) << "RSA:calc: SSS:  old avgspeed rotspeed setspeed"
 << ss_old[ss].avg_speed << ss_old[ss].rotorspeed << ss_old[ss].set_speed;
         int ispeed   = ss_old[ ss ].set_speed;
         ispeed       = ( ispeed < 100 ) ? ss_old[ ss ].rotorspeed : ispeed;
         speeds << ispeed;
      }
DbgLv(1) << "RSA:calc: SSS:   speeds" << speeds;

      simparams.speed_step.resize( nspstep );

      int kscan    = sp0.scans;
      // Create the new full speed step vector by copy or create-from-sim_speed_prof
      for ( int ss = 0; ss < nspstep; ss++ )
      {
         US_SimulationParameters::SimSpeedProf* ipp = &simparams.sim_speed_prof[ ss ];
         int ispeed        = ipp->rotorspeed;
         int ssox          = speeds.indexOf( ispeed );
DbgLv(1) << "RSA:calc: SSS:   ss" << ss << "ispeed" << ispeed << "ssox" << ssox;

         if ( ssox >= 0 )
         {  // If speed in old speed step vector, just copy it
            simparams.speed_step[ ss ] = ss_old[ ssox ];
            if ( ssox == cdsx )  // Adjust pointer to dataset speed
               cdsx           = ss;
DbgLv(1) << "RSA:calc: SSS:     cdsx" << cdsx;
         }

         else
         {  // Speed not in old speed step:  build from sim_speed_prof
            US_SimulationParameters::SpeedProfile* opp = &simparams.speed_step[ ss ];
            // Get values from SimSpeedProf
            double accel      = ipp->acceleration;
            double w2t_eacc   = ipp->w2t_e_accel;
            double w2t_estep  = ipp->w2t_e_step;
            double avg_speed  = ipp->avg_speed;

            int rspeed        = ipp->rotorspeed;
            int duration      = ipp->duration;
            int time_first    = ipp->time_f_scan;
            int time_last     = ipp->time_l_scan;
            int time_eacc     = ipp->time_e_accel;
            int time_estep    = ipp->time_b_accel + duration;
            // Compute values needed by SpeedProfile
            int delay         = time_first - ipp->time_b_accel;
            double dur_mins   = (double)duration / 60.0;
            double dly_mins   = (double)delay / 60.0;
            int dur_hrs       = (int)dur_mins / 60;
            int dly_hrs       = (int)dly_mins / 60;
            dur_mins          = dur_mins - (double)dur_hrs * 60.0;
            dly_mins          = dly_mins - (double)dly_hrs * 60.0;
            double delta_w    = w2t_estep - w2t_eacc;
            double delta_t    = (double)( time_estep - time_eacc );
            double w2t_first  = w2t_eacc + ( time_first - time_eacc )
                                * delta_w / delta_t;
            double w2t_last   = w2t_eacc + ( time_last  - time_eacc )
                                * delta_w / delta_t;

            // Set the values of the SpeedProfile
            opp->duration_minutes  = dur_mins;
            opp->delay_minutes     = dly_mins;
            opp->w2t_first         = w2t_first;
            opp->w2t_last          = w2t_last;
            opp->avg_speed         = avg_speed;
            opp->speed_stddev      = 0.0;
            opp->duration_hours    = dur_hrs;
            opp->delay_hours       = dly_hrs;
            opp->time_first        = time_first;
            opp->time_last         = time_last;
            opp->scans             = kscan;
            opp->rotorspeed        = rspeed;
            opp->acceleration      = accel;
            opp->set_speed         = (int)qRound( (double)rspeed * 0.01 ) * 100;
            opp->acceleration_flag = true;
DbgLv(1) << "RSA:calc: SSS:    tf tl wf wl" << time_first << time_last << w2t_first << w2t_last;
         }
      }

      nstep        = nspstep;
      af_params.cdset_speed = simparams.speed_step[ cdsx ].rotorspeed;
   }  // END:  code to handle speed_step/sim_speed_prof mismatch

   // Flag: any stretch?
   bool strch_rot  = ( simparams.rotorcoeffs[ 0 ] > 0.0  ||
                       simparams.rotorcoeffs[ 1 ] > 0.0 );
   // Flag: multi-speed data?
   bool mspd_data  = false;
   double srpm     = exp_data.scanData[ 0 ].rpm;
   for ( int js = 1; js < exp_data.scanCount(); js++ )
   {
      if ( exp_data.scanData[ js ].rpm != srpm )
      {  // Flag at least one speed change within the raw data
         mspd_data       = true;
         break;
      }
   }
DbgLv(1) << "RSA:calc: strch_rot" << strch_rot << "mspd_data" << mspd_data;

   //----------------------------------------------------------------------------
   // For each component vector find the scans. "af_data" i.e. scans on the
   // experimental grid or simdata i.e. scans on the simulation grid as required.
   // Here size_cv refers to  number of components in the component vector.
   // ---------------------------------------------------------------------------

   for ( int cc = 0; cc < size_cv; cc++ )
   {
#ifdef TIMING_RA
QDateTime clcSt1 = QDateTime::currentDateTime();
#endif
#if 1
#ifndef NO_DB
DbgLv(1) << "RSA:emit ccomp: component" << cc+1;
      emit current_component( cc + 1 );
      qApp->processEvents();
#endif
#endif

      US_Model::SimulationComponent* sc = &system.components[ cc ];
      US_Model::Association*         as;
DbgLv(2)<< "s_n_D_values are" << sc->s << sc->D  << cc ;

      reacting[ cc ] = false;

      for ( int aa = 0; aa <  system.associations.size(); aa++ )
      {
         as  = &system.associations[ aa ];
         for ( int jj = 0; jj < as->rcomps.size(); jj++ )
         {
            if ( cc == (int)as->rcomps[ jj ] )
            {
               reacting[ cc ] = true;
//               current_assoc  = aa;
               break;   // Since a comp appears at most once in an assoc rule
            }
         }
      }

      current_time  = 0.0;
      current_om2t  = 0.0;
      last_time     = 0.0;
      current_speed = 0.0;
      w2t_integral  = 0.0;

      // Clears any previous initial concentration and radius vector
      // and does memory allocations for the vector having radial points
      // and initial concentration vector of size initial_npts.
      CT0.radius       .clear();
      CT0.concentration.clear();
      CT0.radius       .reserve( initial_npts );
      CT0.concentration.reserve( initial_npts );

      // Gap between two radial grid points on the initial grid---
      dr            = ( af_params.current_bottom - af_params.current_meniscus )
                      / (double)( initial_npts - 1 );
      double radval = af_params.current_meniscus;

      // Update the radial grid and concentration vector on the initial grid
      for ( int jj = 0; jj < initial_npts; jj++ )
      {
         CT0.radius        << radval;
         CT0.concentration << 0.0;
         radval       += dr;
      }

      af_c0.radius       .clear();
      af_c0.concentration.clear();

      if ( mspd_data )
      {  // For multi-speed data, use stretch radii just computed
         af_data.radius     = CT0.radius;
      }

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
         for ( int jj = 0; jj < af_data.radius.size(); jj++ )
         {
            scan1.radius        << af_data.radius[ jj ];
            scan1.concentration << scan0->conc   [ jj ];
         }
         US_AstfemMath::interpolate_C0( scan1, af_c0 );
      }

      // Non-reacting case for the components starts here

      if ( ! reacting[ cc ] ) // noninteracting
      {
         initialize_conc( cc, CT0, true ); // Initialize the concentration vector on initial grid

         // Resizes for the s and D values
         af_params.s   .resize( 1 );
         af_params.D   .resize( 1 );
         af_params.kext.resize( 1 );

         // Updates s and D values
         af_params.s   [ 0 ] = sc->s;
         af_params.D   [ 0 ] = sc->D;
         af_params.kext[ 0 ] = sc->extinction * af_params.pathlength;
#ifdef TIMING_RA
totT1+=(clcSt1.msecsTo(QDateTime::currentDateTime()));
#endif
         // Initializes first and last scans, times, omega_square_t and number
         // of speed steps
         int    lscan = 0;    // Used for last scan
         int    fscan = 0;    // Used for first scan
         double time0 = 0.0;  // Last time of the previous speed step
         double time1 = 0.0;  // First time of the current speed step
         double time2 = 0.0;  // Last time of the current speed step
         double omeg0 = 0.0;  // Last omega_square_t of the previous speed step
         double omeg1 = 0.0;  // First omega_square_t of the current speed step
         double omeg2 = 0.0;  // Last omega_square_t of the current speed step
         double next_speed;   // Rotor speed in next speed step
         US_AstfemMath::MfemData* ed;      // Pointer used for af_data i.e. on experimental grid
         is_zero      = false;

         // Calculation for each speed step starts here
         //   'nstep' is the total number of speed_steps in the speed profile
         for ( int speed_step = 0; speed_step < nstep; speed_step++ )
         {
#ifdef TIMING_RA
QDateTime clcSt2 = QDateTime::currentDateTime();
#endif
            sp           = &simparams.speed_step[ speed_step ];
            ed           = &af_data;
            next_speed   = (double)sp->rotorspeed;
            fscan        = 0;
            lscan        = fscan + af_data.scan.count() - 1;
            bool in_step = ( ed->scan[ fscan ].time <= sp->time_last  &&
                             ed->scan[ lscan ].time >= sp->time_first );

            //----------------------------------------------------------------
            // For multi-speed, break out once speed step is beyond data
            //----------------------------------------------------------------
            if ( speed_step > 0  &&  ed->scan[ lscan ].time < sp->time_first )
            {
DbgLv(1) << "RSA:calc: break inside astfem_rsa (step-time beyond last scan-time)" ;
               break;
            }

            time0        = time2;           // Last time of last speed step is assigned
                                            // as first time of the current speed step
                                            // First time is zero if it is the first
                                            // speed step.

            omeg0        = omeg2;           // Last time of w^2_t speed step is assigned
                                            // as first w^2_t of the current speed step
                                            // First w^2_t is zero if it is the first
                                            // speed step.

            // Last time and omega_square_t of the current speed step
            time2        = ( double )( time_end[ speed_step ] );
            omeg2        = w2t_end[ speed_step ];

DbgLv(1)<< "RSA:2-calc: meniscus bottom" << af_params.current_meniscus << af_params.current_bottom;
#if 1
            adjust_limits( sp->rotorspeed ); // Does rotor stretch
#endif
DbgLv(1)<< "RSA:3-calc: meniscus bottom" << af_params.current_meniscus << af_params.current_bottom;

            ed->meniscus = af_params.current_meniscus; // Update meniscus for experimental grid
            ed->bottom   = af_params.current_bottom;   // Update bottom for experimental grid

            if ( mspd_data  &&  strch_rot )
            {  // For multi-speed data, reset speed-appropriate radii
               int points         = ed->radius.count();
               double radinc      = ( ed->bottom - ed->meniscus ) / (double)( points - 1 );
               double radval      = ed->meniscus;
               for ( int jr = 0; jr < points; jr++ )
               {
                  ed->radius[ jr ]   = radval;
                  radval            += radinc;
               }
            }

            // We need to simulate on acceleration zone

            if  ( sp->acceleration_flag ) // Accel flag is used to accelerate the rotor speed.
            {
               // Calculates number of time steps for acceleration simulation grid

               // Number of time steps for acceleration zones is equal to time
               // when acceleration ended minus last time of last timestep. Time
               // when acceleration process ended is obtained from timestate readings.
               // And simulation is done for acceleration zone for each second.

               af_params.time_steps = accel_times[speed_step] - (int)(time0);
               af_params.dt         = 1.0;
               af_params.simpoints  = 2 * simparams.simpoints;
               af_params.start_time = time0 ;
               af_params.start_om2t = omeg0;

               // Simulate for the acceleration zone
               calculate_ni( current_speed, next_speed, speed_step, CT0, simdata, true );

               qApp->processEvents();
               if ( stopFlag ) return 1;

               // Update time and omega_2t
               current_om2t  = accel_w2ts[speed_step] ;
               current_time  = (double) ( accel_times[speed_step] ) ;
               accel_time    = (double) af_params.time_steps;
               time1         = current_time;
               omeg1         = current_om2t;
#ifndef NO_DB
DbgLv(1) << "RSA:emit ctime: eoa time" << current_time << "component" << cc+1;
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

            // Time left to be simulated after acceleration zone
            //  with constant speed
            duration   = time2 - current_time;

            // Insure that the last speed step's simulation extends
            //  well beyond the last experiment scan time
            if ( speed_step == ( nstep - 1 ) )
               duration += (double)( (int)( duration * 0.05 ) );  // +5%

            if ( accel_time > duration )
            {
               DbgErr() << "Attention: acceleration time exceeds duration - "
                           "please check initialization\n";
               return -1;
            }

            // Variables needed to calculate length of time
            // grid on simulation grid
            double omega        = next_speed * M_PI / 30.0;
            af_params.omega_s   = sq( omega );
            double lg_bm_rat    = log( af_params.current_bottom / af_params.current_meniscus );
            double s_omg_fac    = qAbs( sc->s ) * af_params.omega_s;

            // Length of time grid on simulation grid for constant speed zone
            af_params.dt        = lg_bm_rat / ( s_omg_fac * ( simparams.simpoints  ) );
DbgLv(1) << "RSA:calc:  dt" << af_params.dt << "rat omgf simp"
 << lg_bm_rat << s_omg_fac << simparams.simpoints
 << "b m omgs s" << af_params.current_bottom << af_params.current_meniscus
 << af_params.omega_s << sc->s;
            if ( af_params.dt < 0.0 )
               break;

            if ( af_params.dt > duration)
            {
//double dtsv=af_params.dt;
               af_params.dt        = duration;
               af_params.simpoints = 1 + (int)( lg_bm_rat / ( s_omg_fac * af_params.dt ) );

            }

            af_params.time_steps = qCeil ( duration / af_params.dt );

            af_params.start_time = current_time;// Update starting time for constant speed zone
            af_params.start_om2t = current_om2t;// Update starting w2t for constant speed zone

#ifdef TIMING_RA
QDateTime clcSt4 = QDateTime::currentDateTime();
totT3+=(clcSt3.msecsTo(clcSt4));
#endif
//            US_AstfemMath::MfemData* ed = &af_data;

            // Calculate the average of rpms from timestate readings
            //  and use it for constant speed zone
            double avg_speed = simparams.sim_speed_prof[ speed_step ].avg_speed ;

            // Calculate the simulation for constant speed zone
DbgLv(2) <<" CALC:_ni: speed" << avg_speed << "step" << speed_step
 << "sp-men,bot" << simparams.meniscus << simparams.bottom
 << "af-men,bot" << af_params.current_meniscus << af_params.current_bottom
 << "cdset_speed" << af_params.cdset_speed;
            calculate_ni( avg_speed, avg_speed, speed_step, CT0, simdata, false );

            qApp->processEvents();
            if ( stopFlag ) return 1;

            // Set the current time to the last scan of this speed step
            current_time  = time2;
            current_om2t  = omeg2;

#ifdef TIMING_RA
QDateTime clcSt5 = QDateTime::currentDateTime();
totT4+=(clcSt4.msecsTo(clcSt5));
#endif
            // Interpolate the simulated data onto  the experimental time
            // and radius grid, if the experimental time range fits in
            // this speed step.

            if  ( in_step )
            {
               US_AstfemMath::interpolate( af_data, simdata, use_time );
            }

            if ( !simout_flag )
            {
               simdata.radius.clear();
               simdata.scan  .clear();
            }

            // Set the current speed to the constant rotor
            //  speed of the current speed step
            current_speed = next_speed;

#ifndef NO_DB
            qApp->processEvents();
#endif

#ifdef TIMING_RA
totT5+=(clcSt5.msecsTo(QDateTime::currentDateTime()));
#endif

         } // Speed step loop

#if 0
#ifndef NO_DB
DbgLv(1) << "RSA:emit ccomp: component" << cc+1;
         emit current_component( cc + 1 );
         qApp->processEvents();
#endif
#endif
      } // Non-interacting case

   } // Model Component loop

#ifdef TIMING_RA
QDateTime clcSt6 = QDateTime::currentDateTime();
#endif

   //---------------------------------------------
   //-----Interacting part starts from here-------
   //---------------------------------------------

   // Resize af_params.local_index
   af_params.local_index.resize( size_cv );

   US_AstfemMath::ComponentRole cr;

   for ( int group = 0; group < rg.size(); group++ )
   {
      int num_comp = rg[ group ].GroupComponent.size();
      int num_rule = rg[ group ].association.size();
      af_params.rg_index = group;
      af_params.s          .resize( num_comp );
      af_params.D          .resize( num_comp );
      af_params.kext       .resize( num_comp );
      af_params.role       .resize( num_comp );
      af_params.association.resize( num_rule );

      for ( int m = 0; m < num_rule; m++ )
      {
         af_params.association[ m ] = system.associations[ rg[ group ].association[ m ] ];
      }

      for ( int jj = 0; jj < num_comp; jj++ )
      {
         int index = rg[ group ].GroupComponent[ jj ];
 DbgLv(2) << "RSA:    jj index" << jj << index;

         US_Model::SimulationComponent* sc = &system.components[ index ];
         af_params.s   [ jj ] = sc->s;
         af_params.D   [ jj ] = sc->D;
         af_params.kext[ jj ] = sc->extinction * af_params.pathlength;

         // Global to local index
         af_params.local_index[ index ] = jj;

         cr.comp_index        = index;
         cr.assocs .clear();
         cr.stoichs.clear();

         af_params.role[ jj ] =  cr;  // Add jj'th rule

         // Check all assoc rule in this rg
         for ( int aa = 0; aa < rg[ group ].association.size(); aa++ )
         {
            // Check all comp in rule
            int rule   = rg[ group ].association[ aa ];
DbgLv(2) << "RSA:     aa rule" << aa << rule;
            US_Model::Association* as   = &system.associations[ rule ];

            for ( int rr = 0; rr < as->rcomps.size(); rr++ )
            {
DbgLv(2) << "RSA:      rr af-index as-react" << rr
 << af_params.role[jj].comp_index << as->rcomps[rr];
               if ( af_params.role[ jj ].comp_index ==
                    as->rcomps[ rr ] )
               {
                  // local index for the rule
                  af_params.role[ jj ].assocs .append( aa );
                  af_params.role[ jj ].stoichs.append( as->stoichs[ rr ] );
                  break;
               }
            } //  for "rr" loop
         } //  for "aa" loop
      } // for "jj" loop

      for ( int aa = 0; aa < num_rule; aa++ )
      {
         US_Model::Association* as = &af_params.association[ aa ];
         for ( int rr = 0; rr < as->rcomps.size(); rr++ )
         {
            as->rcomps[ rr ] =
               af_params.local_index[ as->rcomps[ rr ] ];
DbgLv(2) << "RSA:     aa rr rcn" << aa << rr << as->rcomps[rr];
         } // for "rr" loop
      } // for "aa" loop

      current_time  = 0.0;
      current_speed = 0.0;
DbgLv(2) << "RSA: (3)AP.start_om2t" << af_params.start_om2t;
//      w2t_integral  = 0.0;
//      last_time     = 0.0;
      w2t_integral  = af_params.start_om2t;
      last_time     = af_params.start_time;

      dr            =
         ( af_params.current_bottom - af_params.current_meniscus ) /
         ( initial_npts - 1 );

      QVector< US_AstfemMath::MfemInitial > vC0Vec( num_comp );
      vC0 = vC0Vec.data();

      for ( int jj = 0; jj < num_comp; jj++ )
      {
         CT0.radius       .clear();
         CT0.concentration.clear();
         CT0.radius       .reserve( initial_npts );
         CT0.concentration.reserve( initial_npts );
         double radval = af_params.current_meniscus;
DbgLv(2) << "RSA:      jj in_npts" << jj << initial_npts;

         for ( int ii = 0; ii < initial_npts; ii++ )
         {
            CT0.radius        << radval;
            CT0.concentration << 0.0;
            radval       += dr;

         }
         rg[ group ].GroupComponent[ jj ] = jj ;//you added

         initialize_conc( rg[ group ].GroupComponent[ jj ], CT0, false );
         vC0[ jj ] = CT0;
      }

      decompose( vC0 );


DbgLv(2) << "RSA: decompose OUT";
#ifdef TIMING_RA
QDateTime clcSt5 = QDateTime::currentDateTime();
#endif

      int    nstep = simparams.speed_step.size();
      int    lscan = 0;
      int    fscan = 0;
      double time0 = 0.0;
//      double time1 = 0.0;
      double time2 = 0.0;
      double step_speed;
      US_SimulationParameters::SpeedProfile* sp;
      US_AstfemMath::MfemData* ed = &af_data;
      simparams.sim    = ( exp_data.channel == 'S' );

      if ( simparams.tsobj == NULL  ||
           simparams.sim_speed_prof.count() < 1 )
      {  // Timestate is not properly loaded
DbgLv(1) << "timestate file does not exist";
#ifdef NO_DB
         QString tmst_fpath = "../" + temp_Id_name() + ".time_state.tmst";

         if ( ! QFile( tmst_fpath ).exists() )
            US_AstfemMath::writetimestate( tmst_fpath, simparams, exp_data );
#else
         QString tmst_fpath = US_Settings::tmpDir() + "/" + temp_Id_name() + ".time_state.tmst";
         US_AstfemMath::writetimestate( tmst_fpath, simparams, exp_data );
#endif
         simparams.simSpeedsFromTimeState( tmst_fpath );
DbgLv(1)<<"after writing timestate file" << simparams.sim_speed_prof.count();
      }

      else
      {  // Timestate exists and is loaded properly
DbgLv(1) << "rsa : timestate file exists and timestateobject = "
 << simparams.tsobj << simparams.sim_speed_prof.count() ;
      }
DbgLv(1) << "SS2: ss size" << simparams.speed_step.size()
 << "ssp size" << simparams.sim_speed_prof.size();

      for ( int step = 0; step < simparams.sim_speed_prof.size(); step++ )
      {  // Fill work time,omega2t vectors with values for each step
         time_end    << simparams.sim_speed_prof[ step ].time_e_step;
         w2t_end     << simparams.sim_speed_prof[ step ].w2t_e_step;
         accel_times << simparams.sim_speed_prof[ step ].time_e_accel;
         accel_w2ts  << simparams.sim_speed_prof[ step ].w2t_e_accel;
DbgLv(1)<< "subha: rsa readings: " << accel_times[step] << accel_w2ts[step];
      }

      if ( simparams.sim_speed_prof.count() > 0 )
      {
//          time2        = qMax( 0.0, (double)simparams.sim_speed_prof[ 0 ].time_b_accel - 1 );
          time2        = (double)simparams.sim_speed_prof[ 0 ].time_b_accel;
      }

      // Walk through each speed step

      for ( int step = 0; step < nstep; step++ )
      {
         sp           = &simparams.speed_step[ step ];
         ed           = &af_data;
         step_speed   = (double)sp->rotorspeed;

#if 1
         adjust_limits( sp->rotorspeed );
#endif

         accel_time   = 0.0;
         fscan        = 0;
         lscan        = fscan + af_data.scan.count() - 1;

         // First time and omega_square_t of the current speed step
         time0        = time2;
         omeg0        = omeg2;

         // Last time and omega_square_t of the current speed step
         time2        = ( double )( time_end[ step ] );
         omeg2        = w2t_end [ step ] ;

         // Update meniscus and bottom for experimental grid
         ed->meniscus = af_params.current_meniscus;
         ed->bottom   = af_params.current_bottom;

         if ( mspd_data  &&  strch_rot )
         {  // For multi-speed data, reset speed-appropriate radii
            int points         = ed->radius.count();
            double radinc      = ( ed->bottom - ed->meniscus )
                                 / (double)( points - 1 );
            double radval      = ed->meniscus;
            for ( int jr = 0; jr < points; jr++ )
            {
               ed->radius[ jr ]   = radval;
               radval            += radinc;
            }
         }

         // For multi-speed, break out once speed step is beyond data
         if ( step > 0  &&  ed->scan[ lscan ].time < sp->time_first  &&
              !mspd_data )
            break;

         // We need to simulate acceleration
         if ( sp->acceleration_flag )
         {
            // Calculates number of time steps for acceleration simulation grid
            af_params.time_steps = accel_times[step] - (int)(time0);
            af_params.dt         = 1.0;
            af_params.simpoints  = 2 * simparams.simpoints;
            af_params.start_time = time0;
            af_params.start_om2t = omeg0;

            // Calculate the simulation for the acceleration zone
            calculate_ra2( current_speed, step_speed, vC0, simdata, true );

            current_om2t  = accel_w2ts[ step ];
            current_time  = (double)accel_times[ step ];
            accel_time    = (double)af_params.time_steps;

#if 0
            // If the speed difference is larger than acceleration rate
            //  then we have at least 1 acceleration step

            af_params.time_steps = (int) ( qAbs( step_speed - current_speed ) / sp->acceleration );

            // Each simulation step is 1 second long in the acceleration phase
            af_params.dt        = 1.0;
            af_params.simpoints = 2 * simparams.simpoints;

            // Use a fixed grid with refinement at both ends and with twice
            //  the number of points
            duration      = time2 - time0;
            current_time  = time0;
            af_params.start_time = current_time;

            calculate_ra2( current_speed, step_speed, vC0, simdata, true );

            // Add the acceleration time:
            accel_time    = af_params.dt * af_params.time_steps;
            current_time += accel_time;
#endif

#ifndef NO_DB
DbgLv(1) << "RSA:emit ctime: accel:current_time" << current_time << "step" << step;
            emit new_time( current_time );
            qApp->processEvents();
#endif

            if ( stopFlag ) return 1;
         }  // End of for acceleration

         // Time left to be simulated after accelaration zone
         duration   = time2 - current_time;

#if 1
         adjust_limits( sp->rotorspeed ); // Does rotor stretch
#endif

         //duration      = time2 - time0;
         //delay         = time1 - time0;
         //duration      = ( sp->duration_hours * 3600.
         //                + sp->duration_minutes * 60. );

         // Insure that the last speed step's simulation extends
         //  well beyond the last experiment scan time
         if ( step == simparams.speed_step.size() - 1 )
            duration += (double)( (int)( duration * 0.05 ) );  // +5%

         if ( accel_time > duration )
         {
            DbgErr() << "Attention: acceleration time exceeds duration - "
                        "please check initialization\n";
            return -1;
         }
         else
         {
            duration -= accel_time;
         }

         double s_max = qAbs( af_params.s[ 0 ] );     // Find the largest s

         for ( int mm = 1; mm < af_params.s.size(); mm++ )
              s_max             = qMax( s_max, qAbs( af_params.s[ mm ] ) );

         af_params.omega_s = sq( step_speed * M_PI / 30. );

DbgLv(2) << "RSA: ***COMPUTED(2) omega_s:" << af_params.omega_s << "step_speed"
 << step_speed << "s.size s_max" << af_params.s.size() << s_max;

         double lg_bm_rat  = log( af_params.current_bottom
                                 / af_params.current_meniscus );
         double s_omg_fac  = af_params.omega_s * s_max;
         af_params.dt      = lg_bm_rat
                             / ( s_omg_fac * ( simparams.simpoints - 1 ) );
DbgLv(1) << "simulation_rsa:  dt" << af_params.dt << "rat omgf simp"
 << lg_bm_rat << s_omg_fac << simparams.simpoints
 << "b m omgs smax" << af_params.current_bottom << af_params.current_meniscus
 << af_params.omega_s << s_max;
double dtsv=af_params.dt;

         if ( af_params.dt > duration )
         {
            af_params.dt        = duration;
            af_params.simpoints = 1 +
               (int)( lg_bm_rat / ( s_omg_fac * af_params.dt ) );
DbgLv(1) << "RSA: ***CORRECTED(2) dt:" << duration << dtsv << af_params.simpoints;
         }

         if ( af_params.simpoints > 10000 )
         {
DbgLv(1) << "RSA: ***COMPUTED simpoints:" << af_params.simpoints
 << "omg2t s_max dt dura" << af_params.omega_s << s_max << af_params.dt
 << duration << "** simpts set to 10000 **";
            af_params.simpoints = 10000;
         }
         else
DbgLv(1) << "RSA: ***COMPUTED simpoints:" << af_params.simpoints;

         // Find out the minimum number of simpoints needed to provide the
         // necessary dt:
         af_params.time_steps = qCeil ( duration / af_params.dt );;
         af_params.start_time = current_time;// Update starting time for constant speed zone
         af_params.start_om2t = current_om2t;// Update starting w2t for constant speed zone
         US_AstfemMath::MfemData* ed = &af_data;

DbgLv(2) << "RSA:   tsteps sttime" << af_params.time_steps << current_time;

         calculate_ra2( step_speed, step_speed, vC0, simdata, false );

         // Set the current time to the last scan of this speed step
         current_time  = time2;
         current_om2t  = omeg2;
DbgLv(2) << "RSA:    current_time" << current_time << "fscan lscan"
 << fscan << lscan << "speed" << step_speed;

         // Interpolate the simulated data onto the experimental
         // time and radius grid

         if ( ed->scan[ fscan ].time <= sp->time_last  &&
              ed->scan[ lscan ].time >= sp->time_first )
         {
            US_AstfemMath::interpolate( *ed, simdata, use_time );
         }

         if ( !simout_flag )
         {
            simdata.radius.clear();
            simdata.scan  .clear();
         }

         // Set the current speed to the constant rotor speed of the
         // current speed step
         current_speed = step_speed;

#ifndef NO_DB
         qApp->processEvents();
#endif
         if ( stopFlag ) return 1;
      } // Speed step loop
#ifdef TIMING_RA
//totT5+=(clcSt5.msecsTo(QDateTime::currentDateTime()));
totT6+=(clcSt5.msecsTo(QDateTime::currentDateTime()));
#endif
   } // RG Group loop

DbgLv(2) << "RSA: Speed step OUT";
#ifdef TIMING_RA
QDateTime clcSt7 = QDateTime::currentDateTime();
totT6+=(clcSt6.msecsTo(clcSt7));
#endif

#ifndef NO_DB
   emit current_component( -1 );
   qApp->processEvents();
#endif

#if 0
   US_AstfemMath::MfemData* ed = &af_data;

   if ( time_correction  &&  !simout_flag )
   {
      int nstep  = simparams.speed_step.size();
      double correction = 0.0;

      // Check each speed step to see if it contains acceleration
      for ( int step = 0; step < nstep; step++ )
      {
         US_SimulationParameters::SpeedProfile*
                                   sp = &simparams.speed_step[ step ];
         US_AstfemMath::MfemData*  ed = &af_data;
         int nscans = ed->scan.size();
DbgLv(1) << "RSA: TimeCorr step" << step << "nscans" << nscans;

         if ( nstep > 1 )
         {  // For multi-speed, skip steps outside experimental scan range
            int fscan  = 0;
            int lscan  = nscans - 1;
DbgLv(1) << "RSA: TimeCorr  nscans fscan lscan" << nscans << fscan << lscan;
DbgLv(1) << "RSA: TimeCorr   stimef stimel" << sp->time_first << sp->time_last
 << "etimef etimel" << ed->scan[ fscan ].time << ed->scan[ lscan ].time;

            if ( ed->scan[ fscan ].time > sp->time_last )
               continue;

            if ( step > 0  &&  ed->scan[ lscan ].time < sp->time_first )
               break;
         }

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
               for ( int ii = 0; ii < nscans; ii++ )
               {
                  xtmp[ ii ] = ed->scan[ ii ].time;
                  ytmp[ ii ] = ed->scan[ ii ].omega_s_t;
               }

               US_Math2::linefit( &xtmp, &ytmp, &slope, &intercept, &sigma,
                                  &correlation, nscans );

               correction = -intercept / slope;
            }
DbgLv(2) << "RSA: TimeCorr   correction" << correction << "nscans" << nscans;

            for ( int ii = 0; ii < nscans; ii++ )
               ed->scan[ ii ].time -= correction;

            // We only need make correction for one speed step
            break;
         }  // END: ( sp->acceleration_flag )

         if ( correction > 0.0 )
            break;
      }  // END: step loop
   }  // END: ( time_correction && !simout_flag )
#endif

DbgLv(2) << "RSA: Time Corr OUT";
#ifdef TIMING_RA
QDateTime clcSt8 = QDateTime::currentDateTime();
totT7+=(clcSt7.msecsTo(clcSt8));
#endif

   if ( !simout_flag )
      store_mfem_data( exp_data, af_data );    // normal experiment grid
   else
      store_mfem_data( exp_data, simdata );    // raw simulation grid

   //for (int i = 0; i< af_data.scan.size(); i++)
   //    qDebug()<< af_data.scan[i].time;

#ifdef TIMING_RA
QDateTime clcSt9 = QDateTime::currentDateTime();
totT8+=(clcSt8.msecsTo(clcSt9));
int elapsedCalc = calcStart.msecsTo( clcSt9 );
ncalls++;
totTC+=elapsedCalc;
if((ncalls%TIMING_RA_INC)<1) {
 DbgLv(1) << "  Elapsed fem-calc ms" << elapsedCalc << "nc totC" << ncalls << totTC << "  size_cv" << size_cv;
 DbgLv(1) << "   t1 t2 t3 t4 t5 t6 t7 t8"
  << totT1 << totT2 << totT3 << totT4 << totT5 << totT6 << totT7 << totT8;
}
#endif

DbgLv(1) << "RSA:calc: ++ ASTFEM CALC DONE ++";
   return 0;
}

//----------------
// Nowhere used !!
//----------------
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
      }

      else if ( ncomp == 3 )
      {
         int stoich3      = as->stoichs[ 2 ];
         as->stoichs[ 0 ] = stoich1 > 0 ? stoich1 : -stoich1;
         as->stoichs[ 1 ] = stoich2 > 0 ? stoich2 : -stoich2;
         as->stoichs[ 2 ] = stoich3 < 0 ? stoich3 : -stoich3;
      }
   }
}

// Adjusts meniscus and bottom position based on rotor coefficients
void US_Astfem_RSA::adjust_limits( int speed )
{
   // First correct meniscus to theoretical position at rest:
   int icset_speed             = (int)af_params.cdset_speed;
   double stretch_value        = stretch( simparams.rotorcoeffs, icset_speed );
DbgLv(1)<< "RSA:adjlim stretch1" << stretch_value << "speed" << af_params.cdset_speed;

   // This is the meniscus at rest
   af_params.current_meniscus  = simparams.meniscus - stretch_value;

   // Calculate rotor stretch at current speed
   stretch_value               = stretch( simparams.rotorcoeffs, speed );
DbgLv(1)<< "RSA:adjlim  stretch2" << stretch_value << "speed" << speed;

   // Add current stretch to meniscus at rest
   af_params.current_meniscus += stretch_value;

#if 0
   // Add current stretch to bottom at rest
   af_params.current_bottom    = simparams.bottom_position + stretch_value;
#endif
#if 1
   // Either use current bottom or add current stretch to bottom at rest
   if ( icset_speed == speed  &&
        simparams.bottom > simparams.bottom_position )
   {  // If the current speed step matches the dataset speed step
      // and a specific bottom was given, then use that bottom value.
      af_params.current_bottom = simparams.bottom;
   }
   else
   {  // If no bottom was given or we are at a speed step preceding the
      // current dataset's, then calculate the bottom value.
      af_params.current_bottom = af_params.bottom_pos + stretch_value;
   }
#endif
}

// Calculates stretch for rotor coefficients array and rpm
double US_Astfem_RSA::stretch( double* rotorcoeffs, int rpm )
{
   double speed    = (double)rpm;
   return ( rotorcoeffs[ 0 ] * speed
          + rotorcoeffs[ 1 ] * sq( speed ) );
}

// Setup reaction groups
void US_Astfem_RSA::initialize_rg( void )
{
   //qDebug()<< "initialize_rg is called" ;
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
         component1 = ( ncomp > 0 ) ? av->rcomps[ 0 ] : -1;
         component2 = ( ncomp > 1 ) ? av->rcomps[ 1 ] : -1;
         component3 = ( ncomp > 2 ) ? av->rcomps[ 2 ] : -1;

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
void US_Astfem_RSA::initialize_conc( int kk, US_AstfemMath::MfemInitial& CT0, bool noninteracting )
{
    US_Model::SimulationComponent* sc = &system.components[ kk ];
    int nval   = af_c0.concentration.size();

    //qDebug()<<"entering_initialize_conc"<< nval;


    // We don't have an existing CT0 concentration vector. Build up the initial
    // concentration vector with constant concentration
    if  ( nval == 0 )
    {
        double mxct = 0.0;
        nval        = CT0.concentration.size();
        if  ( simparams.band_forming )
        {
            // Calculate the width of the lamella
            double angl = simparams.cp_angle   != 0.0 ? simparams.cp_angle   : 2.5;
            double plen = simparams.cp_pathlen != 0.0 ? simparams.cp_pathlen : 1.2;
            double base = sq( af_params.current_meniscus )+ simparams.band_volume * 360.0 / ( angl * plen * M_PI );
            double lamella_width = sqrt( base ) - af_params.current_meniscus;
            // Calculate the spread of the lamella:
           for ( int j = 0; j < nval; j++ )
           {
               base = ( CT0.radius[ j ] - af_params.current_meniscus ) / lamella_width;
               CT0.concentration[ j ] += sc->signal_concentration * exp( -pow( base, 4.0 ) );
if (  j<2||j>(CT0.concentration.size()-3)||j==(CT0.concentration.size()/40))
 DbgLv(2) << "RSA:  j base conc" << j << base << CT0.concentration[j];
if(mxct<CT0.concentration[j]) {mxct=CT0.concentration[j];}
           }
DbgLv(1) << "RSA:BF:  mxct" << mxct << "lamella_width" << lamella_width;
        }
        else  // !simparams.band_forming
        {
            for ( int j = 0; j < nval; j++ )
            {
                CT0.concentration[j] += sc->signal_concentration;
                //qDebug()<<"CT0_conc"<< CT0.concentration[ j ] << sc->signal_concentration  << j << kk ;
if(mxct<CT0.concentration[j]) {mxct=CT0.concentration[j];}
            }
        }
    }
    else  // af_c0.concentration.size() > 0
    {
        if  ( noninteracting )
        {
            // Take the existing initial concentration vector and copy it to the
            // temporary CT0 vector: needs rubber band to make sure meniscus and
            // bottom equal current_meniscus and current_bottom

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
            int    nval  = CT0.concentration.size();
            C.radius       .clear();
            C.concentration.clear();
            C.radius       .reserve( nval );
            C.concentration.reserve( nval );
            double dr  = ( af_params.current_bottom - af_params.current_meniscus )
                      / (double)( nval - 1 );
            double rad = af_params.current_meniscus;
            for ( int j = 0; j < nval; j++ )
            {
                C.radius       .append( rad );
                C.concentration.append( 0.0 );
                rad   += dr;
            }
            US_AstfemMath::interpolate_C0( af_c0, C );
            for ( int j = 0; j < nval; j++ )
                CT0.concentration[ j ] += C.concentration[ j ];
        }
    }
}

//-----------------------------------------
// Calculation for  non-interacting case
//-----------------------------------------
// rpm_start = current rotor speed
// rpm_stop  = rotor speed to be reached after acceleration
// ( If acceleration zone is there then rpm_stop != rpm_start
//   otherwise rpm_start = rpm_stop )
//
// step = speed step counter for the multiple speed profiles
// ( step = 0 for single speed case)
//
// C_init = Initial concentration vector, which needs to be updated
// at the end of the acceleration zone and at the end of each speed
// step.
//
// simdata = structure containing all the informations i.e. scans
// on the simulation grid which is interpolated to experimental grid
// in "calculate" sub_routine.
//
// accel = acceleration flag, it decides whether rotor needs to be
// accelerated or not i.e. accel = false for constant speed case.
int US_Astfem_RSA::calculate_ni( double rpm_start, double rpm_stop, int step,
                                 US_AstfemMath::MfemInitial& C_init,
                                 US_AstfemMath::MfemData& simdata,
                                 bool accel )
{
#ifdef NO_DB
   static int      Nsave  = 0;
   static int      Nsavea = 0;
   static double** CA = NULL;   // stiffness matrix on left hand side
                                // CA[0...Ms-1][0...N-1][4]

   static double** CB = NULL;   // stiffness matrix on right hand side
                                // CB[0...Ms-1][0...N-1][4]

   static double** CA1;         // for matrices used in acceleration
   static double** CA2;
   static double** CB1;
   static double** CB2;
#else
   double** CA = NULL;          // stiffness matrix on left hand side
                                // CA[0...Ms-1][0...N-1][4]

   double** CB = NULL;          // stiffness matrix on right hand side
                                // CB[0...Ms-1][0...N-1][4]
   double** CA1;                // for matrices used in acceleration
   double** CA2;
   double** CB1;
   double** CB2;
#endif
   double*  C0 = NULL;     // C[m][j]: current/next concentration of
                           // m-th component at x_j
   double*  C1 = NULL;     // C[0...Ms-1][0....N-1]:
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

   // Clears the previous simulation grid
   simdata.radius.clear();
   simdata.scan  .clear();

   // Get the initial time and omega_2_t for simulation
   w2t_integral  = af_params.start_om2t;
   last_time     = af_params.start_time;

   // Declares the scan structure
   US_AstfemMath::MfemScan simscan;

   // Generate the adaptive mesh
   xA            = x.data();
   double sw2    = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30.0 );

   QVector< double > nu;
   nu.clear();
   nu .append( sw2 / af_params.D[ 0 ] );
   mesh_gen( nu, simparams.meshType );

   // Refine left hand side (when s > 0) or right hand side (when s < 0)
   //  for acceleration
   if  ( accel )
   {
      int    jx;
      double xc     = sw2 * ( af_params.time_steps * af_params.dt ) / 3.0;

      if  ( af_params.s[ 0 ] > 0 )
      {
         // Radial distance from meniscus how far the boundary will move during
         // this acceleration step (without diffusion)
         xc           += af_params.current_meniscus;
         for ( jx = 0; jx < Nx - 3; jx++ )
            if ( xA[ jx ] > xc ) break;
      }
      else
      {
         xc           += af_params.current_bottom;
         for ( jx = 0; jx < Nx - 3; jx++ )
            if ( xA[ Nx - jx - 1 ] < xc ) break;
      }
      mesh_gen_RefL( jx + 1, 4 * jx );
   }

   //--------------------------------------
   // Initialize the coefficient matrices
   // -------------------------------------
#ifdef NO_DB
   if  ( Nx > Nsave )
   {
      if  ( Nsave > 0 )
      {
         US_AstfemMath::clear_2d( 3, CA );
         US_AstfemMath::clear_2d( 3, CB );
      }
      Nsave         = Nx;
      US_AstfemMath::initialize_2d( 3, Nsave, &CA );
      US_AstfemMath::initialize_2d( 3, Nsave, &CB );
   }
   else
   {
      for ( int ii = 0; ii < 3; ii++ )
      {
         for ( int jj = 0; jj < Nx; jj++ )
         {
            CA[ ii ][ jj ] = 0.0;
            CB[ ii ][ jj ] = 0.0;
         }
      }
   }
#else
   US_AstfemMath::initialize_2d( 3, Nx, &CA );
   US_AstfemMath::initialize_2d( 3, Nx, &CB );
#endif

   // Define the simulation grid type
   bool fixedGrid = ( simparams.gridType == US_SimulationParameters::FIXED );

#ifdef TIMING_NI
clcSt2 = QDateTime::currentDateTime();
ttT1+=(clcSt1.msecsTo(clcSt2));
#endif

   if ( ! accel ) // No acceleration
   {
      sw2 = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30.0 );
      if  ( fixedGrid )
      {
         ComputeCoefMatrixFixedMesh  ( af_params.D[ 0 ], sw2, CA, CB );
      }
      else if ( af_params.s[ 0 ] > 0 )
      {
//qDebug()<<"MovingMeshR";
         ComputeCoefMatrixMovingMeshR( af_params.D[ 0 ], sw2, CA, CB );
      }
      else
      {
//qDebug()<<"MovingMeshL";
         ComputeCoefMatrixMovingMeshL( af_params.D[ 0 ], sw2, CA, CB );
      }
   }
   else // For acceleration
   {
#ifdef NO_DB
      if  ( Nx > Nsavea )
      {
         if  ( Nsavea > 0 )
         {
            US_AstfemMath::clear_2d( 3, CA1 );
            US_AstfemMath::clear_2d( 3, CB1 );
            US_AstfemMath::clear_2d( 3, CA2 );
            US_AstfemMath::clear_2d( 3, CB2 );
         }
         US_AstfemMath::initialize_2d( 3, Nx, &CA1 );
         US_AstfemMath::initialize_2d( 3, Nx, &CA2 );
         US_AstfemMath::initialize_2d( 3, Nx, &CB1 );
         US_AstfemMath::initialize_2d( 3, Nx, &CB2 );
         Nsavea = Nx;
      }
      else
      {
         for ( int ii = 0; ii < 3; ii++ )
         {
            for ( int jj = 0; jj < Nx; jj++ )
            {
               CA1[ ii ][ jj ] = 0.0;
               CA2[ ii ][ jj ] = 0.0;
               CB1[ ii ][ jj ] = 0.0;
               CB2[ ii ][ jj ] = 0.0;
            }
         }
      }
#else
      US_AstfemMath::initialize_2d( 3, Nx, &CA1 );
      US_AstfemMath::initialize_2d( 3, Nx, &CA2 );
      US_AstfemMath::initialize_2d( 3, Nx, &CB1 );
      US_AstfemMath::initialize_2d( 3, Nx, &CB2 );
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
   QVector< double > C0Vec( Nx );
   QVector< double > C1Vec( Nx );
   QVector< double > rhVec( Nx );
   C0 = C0Vec.data(); // Used for concentration vector at current time step
   C1 = C1Vec.data(); // Used for concentration vector at next time step*/

   // Time evolution

   double* right_hand_side = rhVec.data();
#ifdef TIMING_NI
ttT3+=(clcSt3.msecsTo(QDateTime::currentDateTime()));
clcSt3 = QDateTime::currentDateTime();
#endif

   double time_dif    = ( af_params.time_steps > 1  &&  rpm_stop != rpm_start )
                       ? (double)( af_params.time_steps ) : 1.0;


   double rpm_inc     = ( rpm_stop - rpm_start ) / time_dif;// Increment in rotor speed
   double rpm_current = rpm_start; // Update the rotor speed


   int ntsteps        = af_params.time_steps; // Number of time steps

   for ( int jx = 0; jx < Nx; jx++ )
   {
      C0[ jx ] = 0.0;
      C1[ jx ] = 0.0;
   }

   // Clears previous data on simulation grid
   //  and reserves the radial grid and scans
   simdata.scan.clear();
   simdata.scan.reserve( ntsteps );
   simdata.radius.resize( Nx );
   simscan.conc  .resize( Nx );
   double* rA     = simdata.radius.data();

   // Update the radial grid
   for ( int jx = 0; jx < Nx; jx++ )
   {
      rA[ jx ] = xA[ jx ];
   }
DbgLv(1) << "C_ni:  Nx" << Nx << "rA0 rAn" << rA[0] << rA[Nx-1];

   // Interpolate initial concentration vector onto C0 grid-
   US_AstfemMath::interpolate_C0( C_init, C0, x );

   // Determine zero tolerance: factor of total concentration; or
   //   zero (if band-forming), so "is_zero" is never set true
   const double z_toler_factor = 1.0e-10;
   double z_tolerance          = tot_conc * z_toler_factor;
   if  ( simparams.band_forming )
      z_tolerance        = 0.0;

   if ( ! is_zero )
   {  // If not already into virtual-zero scans, test this one
      double rad_last = af_params.current_bottom - 0.04;

      for ( int jr =  C_init.radius.size() - 1; jr > 1; jr-- )
      {  // Look for virtually-zero concentration from last radius back
         if ( C_init.radius[ jr ] < rad_last )
         {
            if ( C_init.concentration[ jr ] < z_tolerance )
            {  // Concentration very low, so flag simulation zero here on out
               is_zero         = true;
            }
DbgLv(1) << "jr" << jr << "C_init.conc[jr]" << C_init.concentration[jr] << "z_toler" << z_tolerance
 << "rpm" << rpm_current << "radius" << C_init.radius[jr] << "is_zero" << is_zero;
            break;
         }
      }
   }

//*DEBUG*
//is_zero=false;
//*DEBUG*
   if ( is_zero )
   {  // All sim scans will be zero: skip time loop and set last scan time
      ntsteps             = 0;
      //ntsteps             = 1;
      //ntsteps             = accel ? ntsteps : qMin( ntsteps, 3 );
      simscan.time        = last_time;
double ss0=qRound(af_params.s[0]*1.e+15)*0.01;
double dd0=qRound(af_params.D[0]*1.e+8)*0.01;
DbgLv(1) << "C_ni:  IS_ZERO !!  1st step time" << last_time << "rpm_stop" << rpm_stop
 << "s D" << ss0 << dd0 << "ntsteps" << ntsteps;
   }

#ifndef NO_DB
   int jti             = ( ntsteps > 100 ) ? ( ntsteps / 100 ) : 1;
#endif
   int ltsteps         = ntsteps - 1;

   // Calculate all time steps
   for ( int jt = 0; jt < ntsteps; jt++ )
   {
      if ( ( jt == 0 )  && ( accel == false ) )
      {
         simscan.rpm         = (int) rpm_start;
         simscan.time        = last_time;
         simscan.omega_s_t   = w2t_integral;
         simscan.temperature = af_data.scan[ 0 ].temperature;

         for ( int jx = 0; jx < Nx; jx++ )
            simscan.conc[ jx ] = C0[ jx ];

         simdata.scan.append( simscan );
      }

      //if ( accel== true )
      //{  simscan.rpm   = simparams.sim_speed_prof[step].rpm_timestate[ jt+2 ] ;
      //   simscan.omega_s_t   = simparams.sim_speed_prof[step].w2t_timestate[ jt+2 ] ;
      //}

#ifdef TIMING_NI
clcSt4 = QDateTime::currentDateTime();
ttT3+=(clcSt3.msecsTo(clcSt4));
#endif

      rpm_current   += rpm_inc; // Update rotor speed

#ifndef NO_DB
//DbgLv(1) << "RSA: EMIT0 rpm_current" << rpm_current << "rpm inc" << rpm_inc;
      emit current_speed( (int)rpm_current );// Update current rotor speed in GUI window
      qApp->processEvents();
#endif

      // Last time for the simulation grid i.e. time for last scan
      // should be equal to last time of the speed profile. Hence
      // the last increment for the time step is adjusted here.
      // Increment = last time of the speed step - time of the
      //             previous scan.

      if ( ( jt == ltsteps )  &&  ( accel == false ) )
      {
         af_params.dt =  simparams.speed_step[step].time_last -  last_time;
      }

      if  ( accel )
      {  // We have acceleration
         double rpm_ratio = sq( rpm_current / rpm_stop );
         for ( int j1 = 0; j1 < 3; j1++ )
         {
            double* CAj  = CA [ j1 ];
            double* CBj  = CB [ j1 ];
            double* CA1j = CA1[ j1 ];
            double* CA2j = CA2[ j1 ];
            double* CB1j = CB1[ j1 ];
            double* CB2j = CB2[ j1 ];
            for ( int j2 = 0; j2 < Nx; j2++ )
            {
               CAj[ j2 ] = CA1j[ j2 ] + rpm_ratio * ( CA2j[ j2 ] - CA1j[ j2 ] );
               CBj[ j2 ] = CB1j[ j2 ] + rpm_ratio * ( CB2j[ j2 ] - CB1j[ j2 ] );
            }
         }
      } // accel based if loop

      if ( accel == false )                            // Constant speed zone
      {
         simscan.rpm       = (int) rpm_current;        // Rotor speed of scan
         simscan.time      = last_time + af_params.dt; // Time of the scan
         w2t_integral     += ( simscan.time - last_time )      // Omega_2_t
                            * sq( rpm_current * M_PI / 30.0 ); //  increment
         simscan.omega_s_t = w2t_integral;             // Omega_2_t of the scan
      }

      else if ( accel == true ) // Reads timestate recordings for rpm and
      {                         //  omega_2t during acceleration zone
         simscan.time      = last_time + af_params.dt ; // Time of the scan
         simscan.rpm       = simparams.sim_speed_prof[ step ].rpm_timestate[ jt ];
         simscan.omega_s_t = simparams.sim_speed_prof[ step ].w2t_timestate[ jt ];
         w2t_integral      = simscan.omega_s_t;         // Omega_2_t
      }

      last_time            = simscan.time;  // Time of the scan used to update
                                            //  omega_2_t for next scan

      simscan.temperature  = af_data.scan[ 0 ].temperature;
#if 0
if (jt<10 || (jt%500)==0 || (jt+10)>ntsteps) {
int kt=(int)qRound(simscan.time) - simparams.sim_speed_prof[step].time_b_accel-1;
DbgLv(1) << "simulation_rsa: kt jt" << kt << jt << "scan_time=" << simscan.time
 << "w2t=" << simscan.omega_s_t << "accel=" << accel
 << "dt=" << af_params.dt << "rpm" << simscan.rpm
 << "tmst: rpm w2t" << simparams.sim_speed_prof[step].rpm_timestate[kt]
 << simparams.sim_speed_prof[step].w2t_timestate[kt];
}
#endif
#ifdef TIMING_NI
clcSt5 = QDateTime::currentDateTime();
ttT4+=(clcSt4.msecsTo(clcSt5));
#endif

      // Sedimentation part:
      // Calculate the right hand side vector

      if ( accel || fixedGrid )
      {
         right_hand_side[ 0 ] = - CB[ 1 ][ 0 ] * C0[ 0 ]- CB[ 2 ][ 0 ] * C0[ 1 ];
         for ( int jx = 1; jx < Nx - 1; jx++ )
         {
            right_hand_side[ jx ] = - CB[ 0 ][ jx ] * C0[ jx - 1 ]
                                    - CB[ 1 ][ jx ] * C0[ jx     ]
                                    - CB[ 2 ][ jx ] * C0[ jx + 1 ];

         }
         int jx = Nx - 1;
         right_hand_side[ jx ] = - CB[ 0 ][ jx ] * C0[ jx - 1 ]
                                 - CB[ 1 ][ jx ] * C0[ jx     ];

      }
      else
      {
         if  ( af_params.s[ 0 ] > 0 )
         {
            right_hand_side[ 0 ] = - CB[ 2 ][ 0 ] * C0[ 0 ];
            right_hand_side[ 1 ] = - CB[ 1 ][ 1 ] * C0[ 0 ]
                                   - CB[ 2 ][ 1 ] * C0[ 1 ];

            for ( int jx = 2; jx < Nx; jx++ )
            {
               right_hand_side[ jx ] = - CB[ 0 ][ jx ] * C0[ jx - 2 ]
                                       - CB[ 1 ][ jx ] * C0[ jx - 1 ]
                                       - CB[ 2 ][ jx ] * C0[ jx     ];
            }
         }
         else
         {
            for ( int jx = 0; jx < Nx - 2; jx++ )
            {
               right_hand_side[ jx ] = - CB[ 0 ][ jx ] * C0[ jx     ]
                                       - CB[ 1 ][ jx ] * C0[ jx + 1 ]
                                       - CB[ 2 ][ jx ] * C0[ jx + 2 ];
//qDebug() << "rhs=" << right_hand_side[jx];
            }
            int jx = Nx - 2;
            right_hand_side[ jx ] = - CB[ 0 ][ jx ] * C0[ jx     ]
                                        - CB[ 1 ][ jx ] * C0[ jx + 1 ];
//qDebug()<<"rhs=     "<<right_hand_side[jx];
            jx = Nx - 1;
            right_hand_side[ jx ] = -CB[ 0 ][ jx ] * C0[ jx ];
//qDebug()<<"rhs=     "<<right_hand_side[jx];
         }
      } // accel || fixedGrid based if loop ends here

#ifdef TIMING_NI
clcSt6 = QDateTime::currentDateTime();
ttT5+=(clcSt5.msecsTo(clcSt6));
#endif

      // Get the concentration vector for next time step
      US_AstfemMath::tridiag( CA[0], CA[1], CA[2], right_hand_side, C1, Nx );

#ifdef TIMING_NI
clcSt7 = QDateTime::currentDateTime();
ttT6+=(clcSt6.msecsTo(clcSt7));
#endif

      // Update the 'C0' vector
      for ( int jx = 0; jx < Nx; jx++ )
      {
         C0[ jx ] = C1[ jx ];
      }

      double* cA     = simscan.conc.data();
      for ( int jx = 0; jx < Nx; jx++ )
         cA[ jx ] = C0[ jx ];

      simdata.scan.append( simscan );

#ifndef NO_DB
      // Show the movie if movie_flag is true
      if  ( show_movie )
      {
         qApp->processEvents();
         if ( stopFlag ) break;
         //emit new_scan( &x, C0 );
         //emit new_time( simscan.time );
         //qApp->processEvents();
         if ( jt < 10  ||  ( jt + 10 ) > ntsteps  ||  ( jt % jti ) == 0 )
         {
            emit new_scan( &x, C0 );
            emit new_time( simscan.time );
            qApp->processEvents();
DbgLv(1) << "RSA:emit ntime: sscn time" << simscan.time << "jt" << jt;
         }
      }
#endif
#ifdef TIMING_NI
clcSt3 = QDateTime::currentDateTime();
ttT7+=(clcSt7.msecsTo(clcSt3));
#endif
   } // 'jt', i.e. 'time step', loop ends here

   // Last concentration vector goes as initial concentration
   // vector for the next speed case i.e. whenever speed changes,
   // either at end of acceleration zone or end of one speed
   // step in case of multiple speed cases.

   // Update the initial concentration vector
   C_init.radius       .resize( Nx );
   C_init.concentration.resize( Nx );

   for ( int jx = 0; jx < Nx; jx++ )
   {
      C_init.radius       [ jx ] = xA[ jx ];
      C_init.concentration[ jx ] = C0[ jx ];
   }
#ifdef TIMING_NI
clcSt8 = QDateTime::currentDateTime();
#endif

#ifndef NO_DB
DbgLv(1) << "RSA:emit ntime: sscn time" << simscan.time;
   emit new_time( simscan.time );
   qApp->processEvents();
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
  << ttTC << totTK << " Nx" << Nx << " acc fixG" << accel << fixedGrid;
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

// Generate adaptive grids for multi-component Lamm equations
void US_Astfem_RSA::mesh_gen( QVector< double >& nu, int MeshOpt )
{
// Here: Nx: Number of points in the ASTFEM
//    m, b: meniscus, bottom
//    nuMax, nuMin = max and min of nu=sw^2/D
//    MeshType: = 0 ASTFEM grid based on all nu (composite in sharp region)
//              = 1 Claverie (uniform), etc,
//              = 2 Exponential mesh (Schuck's form., no refinement at bottom)
//              = 3 input from data file: "mesh_data.dat"
//              = 4 ASTFVM grid (Finite Volume Method)
//              = 10, acceleration mesh (left and right refinement)

// generate the mesh
   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   int    Np = af_params.simpoints;

   x.clear();
   x.reserve( Np * 2 + 2 );

   switch ( MeshOpt )
   {
      //----------------------
      // Mesh Type 0 (default): adaptive mesh based on all nu
      //---------------------

      case (int)US_SimulationParameters::ASTFEM:
         // Adaptive Space Time FE Mesh without left hand refinement
          std::sort( nu.begin(), nu.end() );  // put nu in ascending order

         if ( nu[ 0 ] > 0 )
         {
DbgLv(2)<< "refine in bottom"<< nu[ 0 ] ;
            mesh_gen_s_pos( nu );
         }

         else if ( nu[ nu.size() - 1 ] < 0 )
            mesh_gen_s_neg( nu );

         else       // Some species with s < 0 and some with s > 0
         {
            double bmval  = m;
            double deltbm = ( b - m ) / (double)( Np - 1 );

            for ( int i = 0; i < Np; i++ )
            {
               x .append( bmval );
               bmval += deltbm;
            }
         }
         break;

      case (int)US_SimulationParameters::CLAVERIE:
         // Claverie mesh without left hand refinement

         for ( int i = 0; i < Np; i++ )
            x .append( m + ( b - m ) * i / ( Np - 1 ) );
         break;

      case (int)US_SimulationParameters::MOVING_HAT:
         // Moving Hat (Peter Schuck's Mesh) w/o left hand side refinement

         x .append( m );

         // Standard Schuck grids
         for ( int i = 1; i < Np - 1; i++ )
            x .append( m * pow( b / m, ( i - 0.5 ) / ( Np - 1 ) ) );

         x .append( b );
         break;

      case (int)US_SimulationParameters::USER:
         // User defined mesh generated from data file
         {
            QFile f( US_Settings::appBaseDir() + "/etc/mesh.dat" );

            if ( f.open( QIODevice::ReadOnly ) )
            {
               QTextStream ts( &f );

               while ( ! ts.atEnd() )  x .append( ts.readLine().toDouble() );

               f.close();

               if ( qAbs( x[ 0 ] - m ) > 1.0e7 )
               {
                  DbgErr() << "The meniscus from the mesh file does not"
                     " match the set meniscus - using Claverie Mesh instead\n";
               }

               if ( qAbs( x[ x.size() - 1 ] - b ) > 1.0e7 )
               {
                  DbgErr() << "The cell bottom from the mesh file does not"
                     " match the set cell bottom - using Claverie Mesh"
                     " instead\n";
               }
            }
            else
            {
               DbgErr() << "Could not read the mesh file - "
                           "using Claverie Mesh instead\n";

               for ( int i = 0; i < af_params.simpoints; i++ )
                  x .append( m + ( b - m ) * i / ( Np - 1 ) );
            }
            break;
         }

      case (int)US_SimulationParameters::ASTFVM:
         // Adaptive Space Time Finite Volume Method
         std::sort( nu.begin(), nu.end() );   // put nu in ascending order

         if ( nu[ 0 ] > 0 )
            mesh_gen_s_pos( nu );

         else if ( nu[ nu.size() - 1 ] < 0 )
            mesh_gen_s_neg( nu );

         else       // Some species with s < 0 and some with s > 0
         {
            for ( int i = 0; i < Np; i++ )
               x .append( m + ( b - m ) * i / ( Np - 1 ) );
         }
         break;

      default:
         qDebug() << "undefined mesh option\n";
         break;

   }

   Nx = x.size();
   xA = x.data();

DbgLv(1) << "RSA: ***COMPUTED Nx" << Nx << "simpts" << af_params.simpoints
 << "omg2t" << af_params.omega_s << "s0" << af_params.s[0]
 << "dt" << af_params.dt;
}

// Generate exponential mesh and refine cell bottom (for s>0)
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

   double meniscus = af_params.current_meniscus;
   double bottom   = af_params.current_bottom;
   int    Np       = af_params.simpoints;

   int    IndLayer = 0;         // number of layers for grids in steep region
   double uth      = 1.0 / Np;  // threshold of u for steep region
   double bmsqdf   = ( sq( bottom ) - sq( meniscus ) ) / ( uth * 2.0 );
   double Npm1     = (double)( Np - 1 );
   double Npm2     = Npm1 - 1.0;
   double Npratio  = Npm2 / Npm1;
   double bmratio  = bottom / meniscus;
   double mbratio  = meniscus / bottom;
   double bmrlog   = log( bmratio );
   double k2log    = log( 2.0   );
   double bmNpow   = pow( bmratio, Npratio );
   double bmdiff   = bottom - meniscus * bmNpow;
   const double PIhalf   = M_PI / 2.0;
DbgLv(1) << "RSA:  msg_pos: nu0" << nu[0] << "nu1" << nu[1] << "Np" << Np << "uth" << uth;
DbgLv(1) << "RSA:  msg_pos:  m " << meniscus << " b " << bottom << " bmsqdf" << bmsqdf
 << "af_parms_size" << af_params.s.size();

   for ( int i = 0; i < af_params.s.size(); i++ ) // Markers for steep regions
   {
      double tmp_xc   = bottom - ( 1.0 / ( nu[ i ] * bottom ) )
                                 * log( nu[ i ] * bmsqdf );

      // # of pts for i-th layer
      int tmp_Nf      = (int) ( PIhalf * ( bottom - tmp_xc )
                        * nu[ i ] * bottom / 2.0 + 0.5 ) + 1;

      // Step required by Pac(i) < 1
      tmp_Hstar       = ( bottom - tmp_xc ) / tmp_Nf * PIhalf;

DbgLv(1) << "RSA:  msg_pos: i " << i << "txc" << tmp_xc << "tNf" << tmp_Nf << "tHs" << tmp_Hstar;
      if ( ( tmp_xc > meniscus ) && ( bmdiff > tmp_Hstar ) )
      {
         xc    .append( tmp_xc );
         Nf    .append( tmp_Nf );
         Hstar .append( tmp_Hstar );
         IndLayer++;
      }
   }

   xc .append( bottom );

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
                  double xi    = (double) j / (double) Mp;
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
               y .append( xci + ( bottom - xci ) *
                    sin( j / ( Nf[ i ] - 1.0 ) * PIhalf ) );

               if ( y[ indp - 1 ] > xcip )   break;
            }
         }
      }
   }

   if ( indp < 2 )
   {  // IndLayer==0  or  indp count less than 2
      x .append( meniscus );

      // Add one more point to Schuck's grids
      for ( int kk = 1; kk < Np - 1 ; kk++ )
      { // Schuck's mesh
         x .append( meniscus * pow( bmratio, (double) kk / Npm1 ) );
      }

      x .append( bottom );
   }

   else
   {  // IndLayer>0  and  indp greater than 1
      int NfTotal = indp;
DbgLv(1) << "RSA:  msg_pos: IndL>0  x sz" << x.size();
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
      double Hf     = y[ NfTotal - 2 ] - y[ NfTotal - 1 ];

      // Number of pts in trans region
      int Nm        = (int)( log( bottom / ( Npm1 * Hf ) * bmrlog ) / k2log ) + 1;

      double xa     = y[ NfTotal - 1 ] - Hf * ( pow( 2.0, (double)Nm ) - 1.0 );

      int Js        = (int)( Npm1 * log( xa / meniscus ) / bmrlog );

      // xa is  modified so that y[ NfTotal - Nm ] matches xa exactly
      xa            = meniscus * pow( bmratio, ( (double)Js / (double)Npm1 ) );

      double tmp_xc = y[ NfTotal - 1 ];
      double HL     = xa * ( 1.0 - mbratio );
      double HR     = y[ NfTotal - 2 ] - y[ NfTotal - 1 ];

      int Mp        = (int)( ( tmp_xc - xa ) * 2.0 / ( HL + HR ) ) + 1;

      if ( Mp > 1 )
      {
         double beta   = ( ( HR - HL ) / 2.0 ) * Mp;
         double alpha  = ( tmp_xc - xa ) - beta;

         for ( int jj = Mp - 1; jj > 0; jj-- )
         {
            double xi     = (double) jj / Mp;
            y .append( xa + alpha * xi + beta * sq( xi ) );
         }
      }

      Nm      = Mp;
DbgLv(1) << "RSA:  msg_pos: IndL>0  Js" << Js << "NfTotal" << NfTotal << "Nm" << Nm;
      // Regular region
      x .append( meniscus );
      yary    = y.data();

      for ( int jj = 1; jj <= Js; jj++ )
         x .append( meniscus * pow( bmratio, ( (double)jj / (double)Npm1 ) ) );

      for ( int jj = NfTotal + Nm - 2; jj >=0; jj-- )
         x .append( yary[ jj ] );
DbgLv(1) << "RSA:  msg_pos: IndL>0  y sz" << y.size() << "Mp" << Mp << "Nm" << Nm << "x sz" << x.size();
   }

   xA      = x.data();
int mm=x.size()/2;
int ee=x.size()-1;
DbgLv(1) << "RSA:  mgs_pos: xA sme" << x[0] << x[1] << x[2]
 << x[mm-1] << x[mm] << x[mm+1] << x[mm+2] << x[ee-2] << x[ee-1] << x[ee];
}

// Generate exponential mesh and refine cell meniscus (for  s<0)
void US_Astfem_RSA::mesh_gen_s_neg( const QVector< double >& nu )
{

   const double PIhalf   = M_PI / 2.0;
   const double PIquar   = M_PI / 4.0;
   const double k2log    = log( 2.0 );
   int               jp, Js, Nf, Nm;
   double            xc, xa, Hstar;
   QVector< double > yr, ys, yt;

   x .clear();
   yr.clear();
   ys.clear();
   yt.clear();

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   int    Np = af_params.simpoints;

   double uth     = 1.0 / Np;   // Threshold of u for steep region
   double nu0     = qAbs( nu[ 0 ] );
   double bmrlog  = log( b / m );
   double mbratio = m / b;
   double Npm1    = (double)( Np - 1 );

   x .reserve( Np );
   yr.reserve( Np );
   ys.reserve( Np );
   yt.reserve( Np );

   xc = m + 1. / ( nu0 * m ) * log( ( sq( b ) - sq( m ) ) * nu0 / ( 2. * uth ) );

   Nf = 1 + (int)( ( xc - m ) * nu0 * m * PIquar );

   Hstar = ( xc - m ) / Nf * PIhalf;

   Nm = 1 + (int)( log( m / ( Npm1 * Hstar ) * bmrlog ) / k2log );

   xa = xc + ( pow( 2.0, (double) Nm ) - 1.0 ) * Hstar;

   Js = (int) ( Npm1 * log( b / xa ) / bmrlog + 0.5 );


   // All grid points at exponentials
   yr .append( b );

   // Is there a difference between simparams.meniscus and
   // af_params.current_meniscus??
   for ( jp = 1; jp < ( Np - 1 ); jp++ )  // Add one more point to Schuck's grids
      yr .append( b * pow( simparams.meniscus / b, ( jp - 0.5 ) / Npm1 ) );

   yr .append( m );

   if ( b * ( pow( mbratio, ( Np - 3.5 ) / Npm1 )
            - pow( mbratio, ( Np - 2.5 ) / Npm1 ) ) < Hstar || Nf <= 2 )
   {
      double* yrA = yr.data();

      // No need for steep region
      for ( jp = Np - 1; jp >= 0; jp-- )
      {
         x .append( yrA[ jp ] );
      }

      xA           = x.data();

      DbgErr() << "Use exponential grid only!(1/10000 reported):  Np Nf Nm"
         << Np << Nf << Nm << "m b nu0" << m << b << nu0;
   }
   else
   {
      // Nf > 2
      double xcm  = xc - m;
      double Nfm1 = (double)( Nf - 1 );
      for ( jp = 0; jp < ( Nf - 1 ); jp++ )
         ys .append( xc - xcm * sin( (double)jp / Nfm1 * PIhalf ) );

      ys .append( m );

      for ( jp = 0; jp < Nm; jp++ )
         yt .append( xc + ( pow( 2.0, (double) jp ) - 1.0 ) * Hstar );

      double* ysA = ys.data();
      double* ytA = yt.data();
      double* yrA = yr.data();

      // set x:
      for ( jp = Nf - 1; jp >= 0; jp-- )
         x .append( ysA[ jp ] );

      for ( jp = 1; jp < Nm; jp++ )
         x .append( ytA[ jp ] );

      for ( jp = Js; jp >= 0; jp-- )
         x .append( yrA[ jp ] );

      // Smooth out
      xA           = x.data();
      jp           = Nf + Nm;
      xA[ jp     ] = ( xA[ jp - 1 ] + xA[ jp + 1 ] ) / 2.0;
      xA[ jp + 1 ] = ( xA[ jp     ] + xA[ jp + 2 ] ) / 2.0;
   } // if
}

// Refine mesh near meniscus (for s>0) or near bottom (for s<0)
//                to be used for the acceleration stage
//
//  parameters: N0 = number of elements near meniscus (or bottom) to be refined
//              M0 = number of elements to be used for the refined region
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
      for ( int jp = 0; jp < M0; jp++ )
      {
         double tmp = (double) jp / (double)M0;
         tmp        = 1.0 - cos( tmp * PIhalf );
         zz .append( xA[ 0 ] * ( 1.0 - tmp ) + xA[ N0 ] * tmp );
         //double tmp = 1.0 - cos( ( (double)jp / (double)M0 ) * PIhalf );
         //zz << ( xA[ 0 ] * tmp + xA[ N0 ] * ( 1.0 - tmp ) );
      }

      for ( int jp = N0; jp < x.size(); jp++ )
      //for ( int jp = M0; jp < x.size(); jp++ )
         zz .append( xA[ jp ] );

      x.clear();
      x.reserve( zz.size() );
      zA = zz.data();

      for ( int jp = 0; jp < zz.size(); jp++ )
         x .append( zA[ jp ] );
   }
   else if ( US_AstfemMath::maxval( af_params.s ) < 0 ) //  All species with s<0
   {
      for ( int jp = 0; jp < x.size() - N0; jp++ )
         zz .append( xA[ jp ] );

      // Refine around the bottom for acceleration
      int    kk = x.size() - 1;
      double x1 = xA[ kk - N0 ];
      double x2 = xA[ kk ];
      double tinc = PIhalf / (double)M0;
      double tval = 0.0;

      for ( int jp = 1; jp <= M0; jp++ )
      {
         tval      += tinc;
         double tmp = sin( tval );
         zz .append( x1 * ( 1.0 - tmp ) + x2 * tmp );
      }

      x.clear();
      x.reserve( zz.size() );
      zA         = zz.data();

      for ( int jp = 0; jp < zz.size(); jp++ )
         x .append( zA[ jp ] );
   }
   else                  // Sedimentation and floating mixed up
      DbgErr() << "No refinement at ends since sedimentation "
                  "and floating mixed ...\n" ;

   Nx         = x.size();
   xA         = x.data();
int mm=x.size()/2;
int ee=x.size()-1;
DbgLv(1) << "RSA:mgR: Nx xA sme" << Nx << x[0] << x[1] << x[2]
 << x[mm-1] << x[mm] << x[mm+1] << x[mm+2] << x[ee-2] << x[ee-1] << x[ee];
}

// Compute the coefficient matrices based on fixed mesh
void US_Astfem_RSA::ComputeCoefMatrixFixedMesh(
      double D, double sw2, double** CA, double** CB )
{
   if ( Nx != x.size()  ||  Nx < 1 )
      DbgErr() << "***FixedMesh ERROR*** Nx x.size" << Nx << x.size()
         << " params.s[0] D sw2" << af_params.s[0] << D << sw2;

#ifdef NO_DB
   static int       Nsave = 0;
   static double*** Stif  = NULL;
   xA = x.data();

   if ( Nx > Nsave )
   {
      if ( Nsave > 0 )
         US_AstfemMath::clear_3d( Nsave, 4, Stif );

      Nsave = Nx + 200;
      US_AstfemMath::initialize_3d( Nsave, 4, 4, &Stif );
   }
#else
   double*** Stif  = NULL;
   US_AstfemMath::initialize_3d( Nx, 4, 4, &Stif );
#endif

   double xd[ 4 ][ 2 ];     // coord for vertices of quad elem

   for ( int k = 0; k < Nx - 1; k++ )
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

   for ( k = 1, m = 0; k < Nx - 1; k++, m++ )
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

   // elem[ Nx-2 ]; i=1,2
   k = Nx - 1;
   m = k  - 1;
   CA[ 0 ][ k ]  = Stif[ m ][ 3 ][ 1 ] + Stif[ m ][ 3 ][ 2 ];  // j=3;
   CA[ 1 ][ k ]  = Stif[ m ][ 2 ][ 1 ] + Stif[ m ][ 2 ][ 2 ];  // j=2;
   CB[ 0 ][ k ]  = Stif[ m ][ 0 ][ 1 ] + Stif[ m ][ 0 ][ 2 ];  // j=0;
   CB[ 1 ][ k ]  = Stif[ m ][ 1 ][ 1 ] + Stif[ m ][ 1 ][ 2 ];  // j=1;

#ifndef NO_DB
   US_AstfemMath::clear_3d( Nx, 4, Stif );
#endif
//*DEBUG
int mm=Nx/2;
DbgLv(1) << "RSA:CCMFM: CA0 sme" << CA[0][0] << CA[0][1] << CA[0][2]
 << CA[0][mm-1] << CA[0][mm] << CA[0][mm+1] << CA[0][mm+2]
 << CA[0][Nx-3] << CA[0][Nx-2] << CA[0][Nx-1];
DbgLv(1) << "RSA:CCMFM: CA1 sme" << CA[1][0] << CA[1][1] << CA[1][2]
 << CA[1][mm-1] << CA[1][mm] << CA[1][mm+1] << CA[1][mm+2]
 << CA[1][Nx-3] << CA[1][Nx-2] << CA[1][Nx-1];
DbgLv(1) << "RSA:CCMFM: CB0 sme" << CB[0][0] << CB[0][1] << CB[0][2]
 << CB[0][mm-1] << CB[0][mm] << CB[0][mm+1] << CB[0][mm+2]
 << CB[0][Nx-3] << CB[0][Nx-2] << CB[0][Nx-1];
DbgLv(1) << "RSA:CCMFM: CB1 sme" << CB[1][0] << CB[1][1] << CB[1][2]
 << CB[1][mm-1] << CB[1][mm] << CB[1][mm+1] << CB[1][mm+2]
 << CB[1][Nx-3] << CB[1][Nx-2] << CB[1][Nx-1];
//*DEBUG
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

   if ( Nx > Nsave )
   {
      if ( Nsave > 0 )
         US_AstfemMath::clear_3d( Nsave, 4, Stif );

      Nsave = Nx + 200;
      US_AstfemMath::initialize_3d( Nsave, 4, 4, &Stif );
   }
#else
   double*** Stif  = NULL;
   US_AstfemMath::initialize_3d( Nx, 4, 4, &Stif );
#endif

   // elem[0]: triangle
   xd[ 0 ][ 0 ] = xA[ 0 ];  xd[ 0 ][ 1 ] = 0.;
   xd[ 1 ][ 0 ] = xA[ 1 ];  xd[ 1 ][ 1 ] = af_params.dt;
   xd[ 2 ][ 0 ] = xA[ 0 ];  xd[ 2 ][ 1 ] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ 0 ] );

   // elem[k]: k=1..(Nx-2), quadrilateral
   for ( int k = 1; k < Nx - 1; k++ ) // loop for all elem
   {
      xd[ 0 ][ 0 ] = xA[ k - 1 ];   xd[ 0 ][ 1 ] = 0.0;
      xd[ 1 ][ 0 ] = xA[ k     ];   xd[ 1 ][ 1 ] = 0.0;
      xd[ 2 ][ 0 ] = xA[ k + 1 ];   xd[ 2 ][ 1 ] = af_params.dt;
      xd[ 3 ][ 0 ] = xA[ k     ];   xd[ 3 ][ 1 ] = af_params.dt;
      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // elem[Nx-1]: triangle
   xd[ 0 ][ 0 ] = xA[ Nx - 2 ];   xd[ 0 ][ 1 ] = 0.0;
   xd[ 1 ][ 0 ] = xA[ Nx - 1 ];   xd[ 1 ][ 1 ] = 0.0;
   xd[ 2 ][ 0 ] = xA[ Nx - 1 ];   xd[ 2 ][ 1 ] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ Nx - 1 ] );

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

   for( k = 2; k < Nx - 1; k++ )
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

   k  = Nx - 1;
   mm = k  - 1;
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
   US_AstfemMath::clear_3d( Nx, 4, Stif );
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

   if ( Nx > Nsave )
   {
      if ( Nsave > 0 )
         US_AstfemMath::clear_3d( Nsave, 4, Stif );

      Nsave = Nx + 200;
      US_AstfemMath::initialize_3d( Nsave, 4, 4, &Stif );
   }
#else
   double*** Stif  = NULL;
   US_AstfemMath::initialize_3d( Nx, 4, 4, &Stif );
#endif

   // elem[0]: triangle
   xd[0][0] = xA[0];
   xd[0][1] = 0.0;
   xd[1][0] = xA[1];  xd[1][1] = 0.0;
   xd[2][0] = xA[0];  xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ 0 ] );

   // elem[k]: k=1..(Nx-2), quadrilateral
   for ( int k = 1; k < Nx - 1; k++ )
   {  // loop for all elem
      xd[0][0] = xA[k  ];   xd[0][1] = 0.0;
      xd[1][0] = xA[k+1];   xd[1][1] = 0.0;
      xd[2][0] = xA[k  ];   xd[2][1] = af_params.dt;
      xd[3][0] = xA[k-1];   xd[3][1] = af_params.dt;
      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // elem[Nx-1]: triangle
   xd[0][0] = xA[Nx-1];   xd[0][1] = 0.0;
   xd[1][0] = xA[Nx-1];   xd[1][1] = af_params.dt;
   xd[2][0] = xA[Nx-2];   xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ Nx - 1 ] );

   // assembly coefficient matrices

   int k = 0;
   CA[1][0] = Stif[0][2][0] + Stif[0][2][1] + Stif[0][2][2];
   CB[0][0] = Stif[0][0][0] + Stif[0][0][1] + Stif[0][0][2] ;
   CB[1][0] = Stif[0][1][0] + Stif[0][1][1] + Stif[0][1][2] ;

   CA[1][0]+= Stif[1][3][0] + Stif[1][3][3] ;
   CA[2][0] = Stif[1][2][0] + Stif[1][2][3] ;
   CB[1][0]+= Stif[1][0][0] + Stif[1][0][3] ;
   CB[2][0] = Stif[1][1][0] + Stif[1][1][3] ;

   for ( int k = 1; k < Nx - 2; k++ )
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

   k = Nx - 2;
   // elem k:
   CA[0][k]  = Stif[k  ][3][1] + Stif[k  ][3][2];  // j=3;
   CA[1][k]  = Stif[k  ][2][1] + Stif[k  ][2][2];  // j=2;
   CB[0][k]  = Stif[k  ][0][1] + Stif[k  ][0][2];  // j=0;
   CB[1][k]  = Stif[k  ][1][1] + Stif[k  ][1][2];  // j=1;

   // elem k+1: (triangle)
   CA[1][k] += Stif[k+1][2][0] + Stif[k+1][2][2];  // j=3;
   CA[2][k]  = Stif[k+1][1][0] + Stif[k+1][1][2];  // j=2;
   CB[1][k] += Stif[k+1][0][0] + Stif[k+1][0][2];  // j=0;

   k = Nx - 1;
   // elem[k]: triangle
   CA[0][k]  = Stif[k  ][2][1] ;
   CA[1][k]  = Stif[k  ][1][1] ;
   CB[0][k]  = Stif[k  ][0][1] ;

#ifndef NO_DB
   US_AstfemMath::clear_3d( Nx, 4, Stif );
#endif
}

// Given total concentration of a group of components involved,
// find the concentration of each component by equilibrium condition
void US_Astfem_RSA::decompose( US_AstfemMath::MfemInitial* C0 )
{
   int num_comp = af_params.role.size();

   // Note: all components must be defined on the same radial grids
   int Npts = C0[ 0 ].radius.size();
   //int           size_cv         = system.components.size();
   //DbgLv(2) << "RSA: decompose() num_comp Npts" << num_comp << Npts;

   // Special case:  self-association  n A <--> An
   if ( num_comp == 2 )       // Only 2 components and one association rule
   {
      int    st0     = af_params.association[ 0 ].stoichs[ 0 ];
      int    st1     = af_params.association[ 0 ].stoichs[ 1 ];
      double k_d     = af_params.association[ 0 ].k_d;
      long double k_assoc = ( k_d != 0.0 ) ? ( 1.0 / k_d ) : 0.0;

      // Extinction coefficient for the monomer, corrected for pathlength
      double ext_M   = af_params.kext[ 0 ];
DbgLv(1)<<"extinction_coefficient" << ext_M;

      // K_association converted from signal to molar units
      double k_assoc1= k_assoc ;
      k_assoc       /= pow(ext_M,st0-1);

#if 0
#ifndef NO_DB
      emit current_component( -Npts );
      qApp->processEvents();
#endif
#endif

      for ( int j = 0; j < Npts; j++ )
      {
         double c1 = 0.0;
         double ct = C0[ 0 ].concentration[ j ] + C0[ 1 ].concentration[ j ] ;

         //Get the root of polynomial equation
         double ct1 = ct ;
         ct1       /= pow(ext_M, 1.0);
         double c1r = US_AstfemMath::find_C1_mono_Nmer( st0, k_assoc1, ct1 );
DbgLv(1)<<"total_concentration" << ct1 << "root= " << c1r;

         if ( st0 == 2 && st1 == -1 )                // mono <--> dimer
         {
            c1 = ( sqrt( 1.0 + 4.0 * k_assoc * ct ) - 1.0 ) / ( 2.0 * k_assoc );
DbgLv(1)<<"entering dimer case" << c1  << k_d;
         }

         else if ( st0 == 3 && st1 == -1 )           // mono <--> trimer
         {
            c1 = US_AstfemMath::cube_root( -ct / k_assoc, 1.0 / k_assoc, 0.0 );
DbgLv(1)<<"entering trimer case" << c1  << k_d;
         }

         else if ( st0 > 3 && st1 == -1 )           // mono <--> n-mer
         {
            c1 = US_AstfemMath::find_C1_mono_Nmer( st0, k_assoc, ct );
DbgLv(1)<<"entering polymer case" << c1  << k_d ;
         }
         else
         {
            DbgErr() << "Warning: invalid stoichiometry in decompose()"
                     << "  st0 st1 c1" << st0 << st1 << c1;
            return;
         }

         double c2 = k_assoc * pow( c1, (double)st0 );
         //qDebug()<<"c2=" << c2;


         if ( af_params.role[ 0 ].stoichs[ 0 ] > 0 )    // c1=reactant
         {
            C0[ 0 ].concentration[ j ] = c1 ;
            C0[ 1 ].concentration[ j ] = c2 ;
DbgLv(1)<<"reactant_case" << C0[0].concentration[j] << C0[1].concentration[j]
 << C0[0].concentration[j]+ C0[1].concentration[j];
         }
         else                                           // c1=product
         {
            C0[ 0 ].concentration[ j ] = c2 ;
            C0[ 1 ].concentration[ j ] = c1 ;
            //qDebug()<<"product_case" ;
         }
#if 0
#ifndef NO_DB
         emit current_component( j + 1 );
#endif
#endif
         qApp->processEvents();
         if ( stopFlag )  break;
      }
DbgLv(2) << "RSA:  decompose NCOMP=2 return";
      return;
   }

   // Any no of components case

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
   // using e^(-k_min * Nx * dt ) < 1.e-7

   double k_min    = 1.0e12;

   // Get minimum k
   for ( int i = 0; i < af_params.association.size(); i++ )
   {
      k_min           = qMin( k_min, af_params.association[ i ].k_off );
   }

   k_min           = qMin( k_min, 1.0e-12 );

   // Max number of time steps to get to equilibrium
   const int time_max     = 100;
   double timeStepSize    = - log( 1.0e-7 ) / ( k_min * time_max );

DbgLv(2) << "RSA:  decompose k_min time_max timeStepSize"
 << k_min << time_max << timeStepSize;

   // time loop
#ifndef NO_DB
   emit calc_start( time_max );
//   emit current_component( -time_max );
   qApp->processEvents();
#endif

   for ( int ti = 0; ti < time_max; ti++ )
   {
#ifndef NO_DB
      if ( show_movie  &&  (ti%8) == 0 )
      {
         //DbgLv(2) << "AR: calc_progr ti" << ti;
DbgLv(1) << "RSA:emit calc_p: ti" << ti;
         emit calc_progress( ti );
         qApp->processEvents();
         //US_Sleep::msleep( 10 );
      }
#endif

      ReactionOneStep_Euler_imp( Npts, C1, timeStepSize );

      if ( stopFlag )  break;

      double diff = 0.0;
      double ct   = 0.0;

      for ( int i = 0; i < num_comp; i++ )
      {
         for ( int j = 0; j < Npts; j++ )
         {
            diff        += qAbs( C2[ i ][ j ] - C1[ i ][ j ] );
            ct          += qAbs( C1[ i ][ j ] );
            C2[ i ][ j ] = C1[ i ][ j ];
         }
      }

#ifndef NO_DB
//      emit current_component( ti + 1 );
//      qApp->processEvents();
#endif

      if ( diff < 1.0e-5 * ct )
      {
#ifndef NO_DB
//         int step = ti + 1;
//         emit current_component( -step );
//         emit current_component( step );
//         qApp->processEvents();
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
          C0[ i ].concentration[ j ] = C1[ i ][ j ] ;
   }

   US_AstfemMath::clear_2d( num_comp, C1 );
   US_AstfemMath::clear_2d( num_comp, C2 );
}

// ReactionOneStep_Euler_imp:  implicit Mid-point Euler
void US_Astfem_RSA::ReactionOneStep_Euler_imp( int Npts, double** C1, double timeStep )
{
   int num_comp = af_params.role.size();
DbgLv(2) << "RSA:Eul: Npts timeStep" << Npts << timeStep
 << "num_comp" << num_comp;

   // Special case:  self-association  n A <--> An
   if ( num_comp == 2 )       // only  2 components and one association rule
   {
      double uhat;

      // Current rule used
      int    rule     = rg[ af_params.rg_index ].association[ 0 ];
      int    st0      = system.associations[ rule ].stoichs[ 0 ];
      int    st1      = system.associations[ rule ].stoichs[ 1 ];
      double extn     = af_params.kext[ system.associations[rule].rcomps[0] ];
      double k_d      = system.associations[ rule ].k_d;
      double k_assoc  = ( k_d == 0.0 ) ? 0.0 : ( 1.0 / ( k_d * extn ) );
      double k_off    = system.associations[ rule ].k_off;

      for ( int j = 0; j < Npts; j++ )
      {
         double ct  = C1[ 0 ][ j ] + C1[ 1 ][ j ];
         double dva = timeStep * k_off * k_assoc;
         double dvb = timeStep * k_off + 2.;
         double dvc = timeStep * k_off * ct + 2.0 * C1[ 0 ][ j ];

         if ( st0 == 2 && st1 == (-1) )              // mono <--> dimer
         {
            uhat = 2. * dvc / ( dvb + sqrt( dvb * dvb + 4. * dva * dvc ) );
         }

         else if ( st0 == 3 && st1 == (-1) )         // mono <--> trimer
         {
            uhat = US_AstfemMath::cube_root( -dvc / dva, dvb / dva, 0.0 );
         }

         else if ( st0  > 3 && st1 == (-1) )         // mono <--> n-mer
         {
            uhat = US_AstfemMath::find_C1_mono_Nmer( st0, dva / dvb, dvc / dvb);
         }

         else
         {
            DbgErr() << "Warning: invalid stoichiometry in decompose()";
            return;
         }

         if ( af_params.role[ 0 ].stoichs[ 0 ] > 0 )   // c1=reactant
         {
             C1[ 0 ][ j ] = 2. * uhat - C1[ 0 ][ j ];
             C1[ 1 ][ j ] = ct - C1[ 0 ][ j ];
         }

         else
         {                                             // c1=product
             C1[ 1 ][ j ] = 2. * uhat - C1[ 1 ][ j ];
             C1[ 0 ][ j ] = ct - C1[ 1 ][ j ];
         }
      }
      return;
   }

   // General cases
   const int iter_max = 20;      // maximum number of Newton iteration allowed

   double** A;

   QVector< double >  y0Vec( num_comp );
   QVector< double >  dnVec( num_comp );
   QVector< double >  bbVec( num_comp );
   QVector< double >  y0rVc( num_comp );
   QVector< double >  y1rVc( num_comp );

   y0rVc.fill( 0.0, num_comp );
   y1rVc.fill( 0.0, num_comp );

   double*  y0      = y0Vec.data();
   double*  delta_n = dnVec.data();
   double*  b       = bbVec.data();
   double*  y0_ref  = y0rVc.data();
   double*  y1_ref  = y1rVc.data();

   US_AstfemMath::initialize_2d( num_comp, num_comp, &A );

   for ( int j = 0; j < Npts; j++ )
   {
      double ct       = 0.0;
      double diff_ref = 0.0;

      for ( int i = 0; i < num_comp; i++ )
      {
         y0[ i ]      = C1[ i ][ j ];
         delta_n[ i ] = 0.0;
         ct          += qAbs( y0[ i ] );
         diff_ref    += qAbs( y0_ref[ i ] - y0[ i ] );
      }

      if ( diff_ref < ( ct * 1.e-7 )  ||  diff_ref < 1.e-9 )
      {
         for ( int i = 0; i < num_comp; i++ )
         {
            C1[ i ][ j ]  = y1_ref[ i ];
         }

         continue;
      }

      for ( int iter = 0; iter < iter_max; iter++ ) // Newton iteration
      {
         double diff     = 0.0;

         for ( int i = 0; i < num_comp; i++ )
         {
            y0[ i ]      = C1[ i ][ j ] + delta_n[ i ];
         }

         //qDebug() << "RSA:Eul: j" << j << "iter" << iter << " y0[0]" << y0[0];
         Reaction_dydt( y0, b );                  // b=dy/dt

         //qDebug() << "RSA:Eul:   post-dydt y0[0]" << y0[0] << "b0" << b[0];
         Reaction_dfdy( y0, A );                  // A=df/dy

         for ( int i = 0; i < num_comp; i++ )
         {
            for ( int k = 0; k < num_comp; k++ )
               A[ i ][ k ] *= ( -timeStep );

            A[ i ][ i ] += 2.0;
            //b[ i ]       = - ( 2 * delta_n[ i ] - timeStep * b[ i ] );
            b[ i ]       = b[ i ] * timeStep - delta_n[ i ] * 2.0;
         }

         if ( US_AstfemMath::GaussElim( num_comp, A, b ) == -1 )
         {
            DbgErr() << "Matrix singular in Reaction_Euler_imp: model 12";
            break;
         }
         else
         {
            diff         = 0.0;

            for ( int i = 0; i < num_comp; i++ )
            {
               delta_n[ i ] += b[ i ];
               diff         += qAbs( delta_n[ i ] );
            }
         }

         if ( diff < ( 1.0e-7 * ct ) )  break;
      } // End of Newton iteration;

      for ( int i = 0; i < num_comp; i++ )
      {
         y0_ref[ i ]   = C1[ i ][ j ];
         C1[ i ][ j ] += delta_n[ i ];
         y1_ref[ i ]   = C1[ i ][ j ];
      }
      //qDebug() << "RSA:Eul: j" << j << "ct diff_ref" << ct << diff_ref;

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
      double k_off      = as->k_off;
      double k_assoc    = ( as->k_d != 0.0 ) ? ( 1.0 / as->k_d ) : 0.0;
             k_assoc   /= af_params.kext[ as->rcomps[ 0 ] ];
      double k_on       = k_assoc * k_off;
      double Q_reactant = 1.0;
      double Q_product  = 1.0;
      //DbgLv(2) << "RSA:ReDydt: m k_off k_on" << m << k_off << k_on;

      for ( int n = 0; n < as->rcomps.size(); n++ )
      {
         // local index of the n-th component in assoc[rule]
         int    ind_cn = as->rcomps[ n ] ;

         // stoichiometry of n-th component in the rule
         int    kstoi  = as->stoichs[ n ] ;
         int    react  = ( kstoi < 0 ) ? -1 : 1;
         double rstoi  = (double)qAbs( kstoi );

         // extinction coefficient of n'th component
         double extn   = af_params.kext[ rgp->GroupComponent[ ind_cn ] ];
         //if(m==0)
DbgLv(2) << "RSA:ReDydt:   m n ind_cn" << m << n << ind_cn
 << "rstoi react extn" << rstoi << react << extn;

         if ( react > 0 ) // comp[n] here is reactant
         {
            Q_reactant *= pow( y0[ ind_cn ] / extn, rstoi );
         }
         else             // comp[n] here is product
         {
            Q_product  *= pow( y0[ ind_cn ] / extn, rstoi );
         }
      }

      Q[ m ] = k_on * Q_reactant - k_off * Q_product;
   }

   for ( int i = 0; i < num_comp; i++ )
   {
      yt[ i ] = 0.0;
      US_AstfemMath::ComponentRole* cr = &af_params.role[ i ];

      for ( int m = 0; m < cr->assocs.size(); m++ )
      {
         yt[ i ] -= ( (double)cr->stoichs[ m ] * Q[ cr->assocs[ m ] ] );
      }

      // convert molar into signal concentration
      yt[ i ] *= af_params.kext[ rgp->GroupComponent[ i ] ];
   }
DbgLv(2) << "RSA:ReDydt: yt[0] yt[n]" << yt[0] << yt[num_comp-1];
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
      double k_off   = as->k_off;
      double k_assoc = ( as->k_d != 0.0 ) ? ( 1.0 / as->k_d ) : 0.0;
             k_assoc /= af_params.kext[ as->rcomps[ 0 ] ];
      double k_on    = k_assoc * k_off;

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

         QC[ m ][ j ] = k_on  * Q_reactant * deriv_r
                      - k_off * Q_product  * deriv_p;
      }  // C_j
   }    // m-rule

   for ( int i = 0; i < num_comp; i++ )
   {
      US_AstfemMath::ComponentRole* cr = &af_params.role[ i ];

      for ( int j = 0; j < num_comp; j++ )
      {
         dfdy[ i ][ j ] = 0.0;

         for ( int m = 0; m < cr->assocs.size(); m++ )
         {
            dfdy[ i ][ j ] -= ( (double)cr->stoichs[ m ]
                              *  QC[ cr->assocs[ m ] ][ j ] );
         }

         // convert molar into signal concentration
         dfdy[ i ][ j ] *= af_params.kext[ rgp->GroupComponent[ i ] ];
      }
   }

   US_AstfemMath::clear_2d( num_rule, QC );
}

// This is the SNI version of operator scheme
// This part is for reacting system
int US_Astfem_RSA::calculate_ra2( double rpm_start, double rpm_stop, US_AstfemMath::MfemInitial* C_init,
                                  US_AstfemMath::MfemData& simdata,  bool accel )
{
   int Mcomp = af_params.s.size();

   US_AstfemMath::MfemScan simscan;

   // Generate the adaptive mesh
   QVector< double > nu;
   nu.clear();
   nu.reserve( Mcomp );

   for ( int i = 0; i < Mcomp; i++ )
   {
      double sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
      nu .append( sw2 / af_params.D[ i ] );
   }

   mesh_gen( nu, simparams.meshType );

   simdata.radius.clear();
   simdata.scan  .clear();
   simdata.radius.reserve( Nx );
   simdata.scan  .reserve( Nx );
   xA        = x.data();

DbgLv(1) << "RSA:_ra2: Mcomp Nx" << Mcomp << Nx << "x size" << x.size()
 << "D0 nu0 D1 nu1" << af_params.D[0] << nu[0] << af_params.D[1] << nu[1];

   bool   fixedGrid = ( simparams.gridType == US_SimulationParameters::FIXED );
   double meniscus  = af_params.current_meniscus;
   double bottom    = af_params.current_bottom;
   int    Nt        = af_params.time_steps;

   // Refine left hand side (when s_max>0) or
   // right hand side (when s<0) for acceleration

   // Used for mesh and dt
   double s_max = US_AstfemMath::maxval( af_params.s );
   double s_min = US_AstfemMath::minval( af_params.s );

   if ( accel )
   {
      double xc ;
DbgLv(1) << "RSA:_ra2:(2) Nx" << Nx << "x size" << x.size();

      if ( s_min > 0.0 )            // all sediment towards bottom
      {
         int   j;
         double sw2 = s_max * sq( rpm_stop * M_PI / 30. );
         xc         = meniscus + sw2 * ( Nt * af_params.dt ) / 3.;

         for ( j = 0; j < Nx - 3; j++ )
         {
            if ( xA[ j ] > xc ) break;
         }

         mesh_gen_RefL( j + 1, 4 * j );
DbgLv(1) << "RSA:_ra2: s_min s_max" << s_min << s_max << "xc xAj"
 << xc << xA[j] << "j" << j << "N0 M0" << (j+1) << (j*4);
      }

     else if ( s_max < 0.0 )    // all float towards meniscus
      {
         // s_min corresponds to fastest component
         int   j;
         double sw2 = s_min * sq( rpm_stop * M_PI / 30. );
         xc         = bottom + sw2 * ( Nt * af_params.dt ) / 3.;

         for ( j = Nx - 1; j > 1; j-- )
         {
            if ( xA[ j ] < xc )  break;
         }

         mesh_gen_RefL( j + 1, 4 * j );
      }
      else
      {
         DbgErr() << "Multicomponent system with sedimentation and "
                     "floating mixed, use uniform mesh";
      }
DbgLv(1) << "RSA:_ra2:(3) Nx" << Nx << "x size" << x.size();
   }
DbgLv(1) << "RSA:_ra2:(4) Nx" << Nx << "x size" << x.size();

   for ( int i = 0; i < Nx; i++ )
      simdata.radius .append( xA[ i ] );

   // Stiffness matrix on left hand side
   // CA[0...Ms-1][4][0...Nx-1]
   double*** CA;
   double*** CA1;
   double*** CA2;

   // Stiffness matrix on right hand side
   // CB[0...Ms-1][4][0...Nx-1]
   double*** CB;
   double*** CB1;
   double*** CB2;

   // Initialize the coefficient matrices
   US_AstfemMath::initialize_3d( Mcomp, 4, Nx, &CA );
   US_AstfemMath::initialize_3d( Mcomp, 4, Nx, &CB );

   if ( accel ) //  Acceleration, so use fixed grid
   {
      US_AstfemMath::initialize_3d( Mcomp, 3, Nx, &CA1 );
      US_AstfemMath::initialize_3d( Mcomp, 3, Nx, &CA2 );
      US_AstfemMath::initialize_3d( Mcomp, 3, Nx, &CB1 );
      US_AstfemMath::initialize_3d( Mcomp, 3, Nx, &CB2 );

      for( int i = 0; i < Mcomp; i++ )
      {
         double sw2 = 0.0;
         ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA1[ i ], CB1[ i ] );

         sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
         ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA2[ i ], CB2[ i ] );
      }
DbgLv(1) << "RSA:_ra2:(5) Nx" << Nx << "x size" << x.size();
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
            double sqb       = sq( af_params.current_bottom );

            for ( int i = 0; i < Mcomp; i++ )
            {
               double sw2   = af_params.s[ i ] * stop_fact;
               double sw2D  = 0.5 * sw2 / af_params.D[ i ];
               double s_rat = af_params.s[ i ] / s_max;
DbgLv(1) << "RSA: smin>0:GlStf: i" << i << "Si Di sw2 sw2D sqb s_max"
 << af_params.s[i] << af_params.D[i] << sw2 << sw2D << sqb << s_max;

               // Grid for moving adaptive FEM for faster sedimentation

               QVector< double > xbvec;
               xbvec.clear();
               xbvec.reserve( Nx );
               xbvec << af_params.current_meniscus;

               for ( int j = 1; j < Nx; j++ )
               {
                  double dval  = 0.1 * exp( sw2D * ( sq( 0.5 *
                                 ( xA[ j - 1 ] + xA[ j ] ) ) - sqb ) );

                  double alpha = s_rat * ( 1.0 - dval ) + dval;
                  xbvec << ( pow( xA[ j - 1 ], alpha ) *
                             pow( xA[ j     ], ( 1.0 - alpha ) ) );
                  //if((j+1)==Nx) xbvec[j]=af_params.current_bottom;
                  if(j<4 || j==(Nx/2) || (j+4)>Nx)
DbgLv(1) << "RSA: smin>0:GlStf:   j" << j << "dval alpha xbj xAj"
 << dval << alpha << xbvec[j] << xA[j];
               }

               double* xb   = xbvec.data();

               GlobalStiff( xb, CA[ i ], CB[ i ], af_params.D[ i ], sw2 );
DbgLv(1) << "RSA: smin>0:GlStf: CA[i]:" << CA[i][0][0] << CA[i][1][0]
 << CA[i][2][Nx-1];
DbgLv(1) << "RSA: smin>0:GlStf: CB[i]:" << CB[i][0][0] << CB[i][1][0]
 << CB[i][2][Nx-1];
            }
         }

         else if ( s_max < 0)    // all components floating
         {
            DbgErr() << "all components floating, not implemented yet";
            return -1;
         }

         else     // sedimentation and floating mixed
         {
            DbgErr() << "sedimentation and floating mixed, suppose use "
                        "fixed grid!";
            return -1;
         }
      } // moving mesh
   } // acceleration

   // Initial condition
   double** C0; // C[m][j]: current/next concentration of m-th component at x_j
   double** C1; // C[0...Ms-1][0....Nx-1]:

DbgLv(1) << "RSA:_ra2:(6) Nx" << Nx << "x size" << x.size();
   US_AstfemMath::initialize_2d( Mcomp, Nx, &C0 );
   US_AstfemMath::initialize_2d( Mcomp, Nx, &C1 );

   // Here we need the interpolate the initial partial
   // concentration onto new grid x[j]

   for( int ii = 0; ii < Mcomp; ii++ )
   {
      // Interpolate the given C_init vector on the new C0 grid
      US_AstfemMath::interpolate_C0( C_init[ ii ], C0[ ii ], x );
   }

   // Total concentration at current and next time step
DbgLv(1) << "RSA: newX3 Nx" << Nx << "C0[00] C0[m0] C0[0n] C0[mn]"
 << C0[0][0] << C0[Mcomp-1][0] << C0[0][Nx-1] << C0[Mcomp-1][Nx-1];

   QVector< double > CT0vec( Nx );
   QVector< double > CT1vec( Nx );
   QVector< double > rhVec ( Nx );
   double* CT0 = CT0vec.data();
   double* CT1 = CT1vec.data();

   for ( int jj = 0; jj < Nx; jj++ )
   {
       CT0[ jj ] = 0.0;

       for ( int ii = 0; ii < Mcomp; ii++ )
          CT0[ jj ] += C0[ ii ][ jj ];

       CT1[ jj ] = CT0[ jj ];
   }
DbgLv(1) << "RSA: newX3  CT0 CTn" << CT1[0] << CT1[Nx-1];

   // Time evolution
   double* right_hand_side = rhVec.data();
#ifndef NO_DB
   int     stepinc = 1000;
   int     stepmax = ( Nt + 2 ) / stepinc + 1;
   bool    repprog = stepmax > 1;

   if ( repprog )
   {
//      emit current_component( -stepmax );
//      emit current_component( 0 );
//      qApp->processEvents();
   }
#endif

DbgLv(1) << "RSA: (5)AP.start_om2t" << af_params.start_om2t;
   last_time       = af_params.start_time;

   for ( int kkk = 0; kkk < Nt + 2; kkk += 2 )   // two steps in together
   {
      double rpm_current = rpm_start + ( rpm_stop - rpm_start ) * ( kkk + 0.5 ) / Nt;

#ifndef NO_DB
DbgLv(1) << "RSA: EMIT rpm_current" << rpm_current << "rpm start/stop" << rpm_start << rpm_stop;
      emit current_speed( (int)rpm_current );
      qApp->processEvents();
#endif

      simscan.time      = af_params.start_time + kkk * af_params.dt;
      simscan.rpm       = (int) rpm_current;

      w2t_integral     += ( ( simscan.time - last_time )* sq( rpm_current * M_PI / 30 ) );

      last_time         = simscan.time;
      simscan.omega_s_t = w2t_integral;

DbgLv(2) << "TMS:RSA:ra: time omegast" << simscan.time << simscan.omega_s_t
 << "step-scan" << simdata.scan.size();

      simscan.conc.clear();
      simscan.conc.reserve( Nx );
DbgLv(2) << "RSA: Nx CT0size" << Nx << CT0vec.size() << "step-scan"
 << simdata.scan.size() << "rss-now" << US_Memory::rss_now();

      for ( int jj = 0; jj < Nx; jj++ )
         simscan.conc.append( CT0[ jj ] );

DbgLv(2) << "TMS:RSA:ra:  kkk" << kkk << "CT0[0] CT0[n]"
 << CT0[0] << CT0[Nx-1] << "accel fixedGrid" << accel << fixedGrid;

      simdata.scan.append( simscan );

      // First half step of sedimentation:

      if ( accel ) // need to reconstruct CA and CB by linear interpolation
      {
         double dval = sq( rpm_current / rpm_stop );

         for ( int i = 0; i < Mcomp; i++ )
         {
            for ( int j1 = 0; j1 < 3; j1++ )
            {
               for ( int j2 = 0; j2 < Nx; j2++ )
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

            for ( int j = 1; j < Nx - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = Nx - 1;

            right_hand_side[ j ] = -CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                  - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::tridiag( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ],
                                    right_hand_side, C1[ i ], Nx );
         }
      }

      else // Moving grid
      {
//DbgLv(2) << "TMS:RSA:ra:   (0)MovGrid: C0[00] C0[0n] C0[m0] C0[1n]"
// << C0[0][0] << C0[0][Nx-1] << C0[Mcomp-1][0] << C0[Mcomp-1][Nx-1];
//DbgLv(2) << "TMS:RSA:ra:   (0)MovGrid: C1[00] C1[0n] C1[m0] C1[1n]"
// << C1[0][0] << C1[0][Nx-1] << C1[Mcomp-1][0] << C1[Mcomp-1][Nx-1];
         for ( int i = 0; i < Mcomp; i++ )
         {
            // Calculate the right hand side vector
            right_hand_side[ 0 ] = - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 3 ][ 0 ] * C0[ i ][ 1 ];

            right_hand_side[ 1 ] = - CB[ i ][ 1 ][ 1 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 2 ][ 1 ] * C0[ i ][ 1 ]
                                   - CB[ i ][ 3 ][ 1 ] * C0[ i ][ 2 ];

            for ( int j = 2; j < Nx - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 3 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = Nx - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                   - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ];

DbgLv(2) << "TMS:RSA:ra:   MovGrid:  i" << i << "CB[0]" << CB[i][2][0]
 << CB[i][3][0] << CB[i][1][1] << CB[i][2][1] << CB[i][3][1];
DbgLv(2) << "TMS:RSA:ra:   MovGrid:   i" << i << "CA[0]" << CA[i][0][0]
 << CA[i][1][0] << CA[i][0][0] << CA[i][1][0] << CA[i][2][0];

            US_AstfemMath::QuadSolver( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ],
                                       CA[ i ][ 3 ], right_hand_side, C1[ i ],
                                       Nx );
DbgLv(2) << "TMS:RSA:ra:   MovGrid:    i" << i << "C1[i]" << C1[i][0]
 << C1[i][Nx-4] << C1[i][Nx-3] << C1[i][Nx-2] << C1[i][Nx-1];
DbgLv(2) << "TMS:RSA:ra:   MovGrid:    i" << i << "C0[i]" << C0[i][0]
 << C0[i][Nx-4] << C0[i][Nx-3] << C0[i][Nx-2] << C0[i][Nx-1];
         }
         //DbgLv(2) << "TMS:RSA:ra:   MovGrid: CA[000] rs[0]" << CA[0][0][0]
         // << right_hand_side[0] << "CB[000]" << CB[0][0][0];
         //DbgLv(2) << "TMS:RSA:ra:   MovGrid: C1[00] C1[0n] C1[m0] C1[1n]"
         // << C1[0][0] << C1[0][Nx-1] << C1[Mcomp-1][0] << C1[Mcomp-1][Nx-1];
      }

      // Reaction part: instantaneous reaction at each node
      //
      // instantaneous reaction at each node
      // [C1]=ReactionOneStep_inst(C1);
      //
      // Finite reaction rate: linear interpolation of instantaneous reaction

DbgLv(2) << "TMS:RSA:ra:   C1[10] C1[11] C1[1k] C1[1n]"
 << C1[1][0] << C1[1][1] << C1[1][Nx-2] << C1[1][Nx-1];

      ReactionOneStep_Euler_imp( Nx, C1, 2 * af_params.dt );

DbgLv(2) << "TMS:RSA:ra(2):   C1[10] C1[11] C1[1k] C1[1n]"
 << C1[1][0] << C1[1][1] << C1[1][Nx-2] << C1[1][Nx-1];

      // For next half time-step in SNI operator splitting scheme

      for ( int j = 0; j < Nx; j++ )
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

      rpm_current = rpm_start + ( rpm_stop - rpm_start ) * ( kkk + 1.5 ) / Nt;

      if ( accel ) // Need to reconstruct CA and CB by linear interpolation
      {
         double dval = sq( rpm_current / rpm_stop );

         for ( int i = 0; i < Mcomp; i++ )
         {
            for ( int j1 = 0; j1 < 3; j1++ )
            {
               for ( int j2 = 0; j2 < Nx; j2++ )
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

            for ( int j = 1; j < Nx - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = Nx - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::tridiag( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ],
                                    right_hand_side, C1[ i ], Nx );
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

            for ( int j = 2; j < Nx - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 3 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = Nx - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                   - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::QuadSolver( CA[ i ][ 0 ], CA[ i ][ 1 ],
                                       CA[ i ][ 2 ], CA[ i ][ 3 ],
                                       right_hand_side, C1[ i ], Nx );
         }
         //DbgLv(2) << "TMS:RSA:ra:   MovGrid: C1[00] C1[0n] C1[m0] C1[1n]"
         // << C1[0][0] << C1[0][Nx-1] << C1[Mcomp-1][0] << C1[Mcomp-1][Nx-1];
      }

      // End of 2nd half step of sedimentation

      // For next 2 time steps

      for ( int j = 0; j < Nx; j++ )
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
DbgLv(1) << "RSA:emit ntime: sstime" << simscan.time;
         emit new_time( simscan.time );
         qApp->processEvents();
      }

      if ( repprog  &&  ( ( kkk + 1 ) & stepinc ) == 0 )
      {
//         emit current_component( ( kkk + 1 ) / stepinc );
//         qApp->processEvents();
      }
#endif

   } // time loop

#ifndef NO_DB
DbgLv(1) << "RSA:emit ntime: sstime" << simscan.time;
   emit new_time( simscan.time );
   qApp->processEvents();
#endif

DbgLv(2) << "TMS:RSA:ra2: Nx" << Nx;
   for ( int i = 0; i < Mcomp; i++ )
   {
     C_init[ i ].radius       .clear();
     C_init[ i ].concentration.clear();
     C_init[ i ].radius       .reserve( Nx );
     C_init[ i ].concentration.reserve( Nx );

     for ( int j = 0; j < Nx; j++ )
     {
        C_init[ i ].radius        .append( xA[ j ] );
        C_init[ i ].concentration .append( C1[ i ][ j ] );
     }
DbgLv(2) << "TMS:RSA:ra2: comp:" << i << "C1[i]:" << C1[i][0] << C1[i][1]
 << C1[i][Nx/2] << C1[i][Nx-3] << C1[i][Nx-2] << C1[i][Nx-1];
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

   US_AstfemMath::initialize_3d( Nx, 6, 2, &Stif );

   // 1st elem
   vx[ 0 ] = x [ 0 ];
   vx[ 1 ] = x [ 1 ];
   vx[ 2 ] = x [ 0 ];
   vx[ 3 ] = x [ 1 ];
   vx[ 4 ] = x [ 2 ];
   vx[ 5 ] = xb[ 1 ];
   US_AstfemMath::IntQT1( vx, D, sw2, Stif[ 0 ], af_params.dt );

   // elems in middle
   for ( int i = 1; i < Nx - 2; i++ )
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
   vx[ 0 ] = x [ Nx - 3 ];
   vx[ 1 ] = x [ Nx - 2 ];
   vx[ 2 ] = x [ Nx - 1 ];
   vx[ 3 ] = x [ Nx - 2 ];
   vx[ 4 ] = x [ Nx - 1 ];
   vx[ 5 ] = xb[ Nx - 2 ];
   vx[ 6 ] = xb[ Nx - 1 ];

   US_AstfemMath::IntQTn2( vx, D, sw2, Stif[ Nx - 2 ], af_params.dt );

   // last elems
   vx[ 0 ] = x [ Nx - 2 ];
   vx[ 1 ] = x [ Nx - 1 ];
   vx[ 2 ] = x [ Nx - 1 ];
   vx[ 3 ] = xb[ Nx - 1 ];
   US_AstfemMath::IntQTn1 ( vx, D, sw2, Stif[ Nx - 1 ], af_params.dt );

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
   for (  int i = 2; i < Nx - 2; i++ )
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
   int i = Nx - 2;
   ca[ 0 ][ i ] = Stif[ i - 1 ][ 3 ][ 1 ];
   ca[ 1 ][ i ] = Stif[ i - 1 ][ 4 ][ 1 ] + Stif[ i ][ 3 ][ 0 ];
   ca[ 2 ][ i ] = Stif[ i - 1 ][ 5 ][ 1 ] + Stif[ i ][ 4 ][ 0 ];
   ca[ 3 ][ i ] = 0.0;

   cb[ 0 ][ i ] = Stif[ i - 1 ][ 0 ][ 1 ];
   cb[ 1 ][ i ] = Stif[ i - 1 ][ 1 ][ 1 ] + Stif[ i ][ 0 ][ 0 ];
   cb[ 2 ][ i ] = Stif[ i - 1 ][ 2 ][ 1 ] + Stif[ i ][ 1 ][ 0 ];
   cb[ 3 ][ i ] =                           Stif[ i ][ 2 ][ 0 ];

   // i=n+1
   i = Nx - 1;
   ca[ 0 ][ i ] = Stif[ i - 1 ][ 3 ][ 1 ];
   ca[ 1 ][ i ] = Stif[ i - 1 ][ 4 ][ 1 ] + Stif[ i ][ 2 ][ 0 ];
   ca[ 2 ][ i ] = 0.0;
   ca[ 3 ][ i ] = 0.0;

   cb[ 0 ][ i ] = Stif[ i - 1 ][ 0 ][ 1 ];
   cb[ 1 ][ i ] = Stif[ i - 1 ][ 1 ][ 1 ] + Stif[ i ][ 0 ][ 0 ];
   cb[ 2 ][ i ] = Stif[ i - 1 ][ 2 ][ 1 ] + Stif[ i ][ 1 ][ 0 ];
   cb[ 3 ][ i ] = 0.0;

   US_AstfemMath::clear_3d( Nx, 6, Stif );
}

void US_Astfem_RSA::load_mfem_data( US_DataIO::RawData&      edata,
                                    US_AstfemMath::MfemData& fdata )
{
   int  nscan  = edata.scanCount();
   int  nconc  = edata.pointCount();
DbgLv(2) << "RSA:f nscan nconc" << nscan << nconc;

   fdata.id       = edata.description;
   fdata.cell     = edata.cell;
   fdata.scan  .resize( nscan );
   //fdata.radius.resize( nconc );
   fdata.radius   = edata.xvalues;
DbgLv(2) << "RSA:f  r0 rn" << fdata.radius[0] << fdata.radius[nconc-1];

   for ( int ii = 0; ii < nscan; ii++ )
   {
      US_AstfemMath::MfemScan* fscan = &fdata.scan[ ii ];

      fscan->temperature = edata.scanData[ ii ].temperature;
      fscan->rpm         = edata.scanData[ ii ].rpm;
      fscan->time        = edata.scanData[ ii ].seconds;
      fscan->omega_s_t   = edata.scanData[ ii ].omega2t;
      fscan->conc        = edata.scanData[ ii ].rvalues;
if ( ii<3 )
 DbgLv(2) << "RSA:f  ii c0 cn" << ii << fscan->conc[0] << fscan->conc[nconc-1]
  << "d0 dn" << edata.scanData[ii].rvalues[0] << edata.scanData[ii].rvalues[nconc-1];
#if 1
   }
#endif
#if 0
      fscan->conc.resize( nconc );

      for ( int jj = 0; jj < nconc; jj++ )
      {
         fscan->conc[ jj ] = edata.value( ii, jj );
      }
   }

   for ( int jj = 0; jj < nconc; jj++ )
   {
      fdata.radius[ jj ] = edata.radius( jj );
   }
#endif
DbgLv(2) << "RSA:f sc0 temp" << fdata.scan[0].temperature;
DbgLv(2) << "RSA:e sc0 temp" << edata.scanData[0].temperature;
}


// Stores simulated data into experimental data
void US_Astfem_RSA::store_mfem_data( US_DataIO::RawData&      edata,
                                     US_AstfemMath::MfemData& fdata )
{
   int  nscan  = fdata.scan.size();
   int  nconc  = fdata.radius.size();
   int  escan  = edata.scanCount();
   int  econc  = edata.pointCount();
DbgLv(2) << "RSA:st_md: nscan nconc" << nscan << nconc;
DbgLv(2) << "RSA:st_md: escan econc" << escan << econc;

   if ( escan != nscan )
      edata.scanData.resize( nscan );

   edata.description = fdata.id;
   edata.cell        = fdata.cell;
   edata.xvalues     = fdata.radius;

   for ( int ii = 0; ii < nscan; ii++ )
   {
      US_AstfemMath::MfemScan* fscan = &fdata.scan    [ ii ];
      US_DataIO::Scan*         escan = &edata.scanData[ ii ];

      escan->temperature = fscan->temperature;
      escan->rpm         = fscan->rpm;
      escan->seconds     = fscan->time;
      escan->omega2t     = fscan->omega_s_t;
      escan->plateau     = fdata.radius[ nconc - 1 ];
      escan->rvalues     = fscan->conc;
   }

DbgLv(2) << "RSA:o-f sc0 temp" << fdata.scan[0].temperature;
DbgLv(2) << "RSA:o-e sc0 temp" << edata.scanData[0].temperature;
}

// Prints speed profile structure
void US_Astfem_RSA::printspeedprofile( US_SimulationParameters::SpeedProfile& sp )
{
     DbgLv(1) << "---------------Speed profile parameters begins here------------------";
     DbgLv(1) << "speed profile";
     DbgLv(1) << "duration_hours " << sp.duration_hours;
     DbgLv(1) << "duration_minutes " << sp.duration_minutes;
     DbgLv(1) << "delay_hours " << sp.delay_hours;
     DbgLv(1) << "delay_minutes " << sp.delay_minutes;
     DbgLv(1) << "scans " << sp.scans;
     DbgLv(1) << "acceleration " << sp.acceleration;
     DbgLv(1) << "rotorspeed " << sp.rotorspeed;
     DbgLv(1) << "acceleration_flag " << sp.acceleration_flag;
     DbgLv(1) <<"speedprofile time first and last" << sp.time_first<< sp.time_last;
     DbgLv(1) <<"omega2t first and last" << sp.w2t_first<< sp.w2t_last;
     DbgLv(1) << "----------------Speed profile parameters ends here--------------------";
}

// Prints simulation components
void US_Astfem_RSA::printsimComponent( US_Model::SimulationComponent& sc )
{
   DbgLv(1) << "---------------Simulation component parameters begins here----------------";
   DbgLv(1) << "molar_concentration" << sc.molar_concentration;
   DbgLv(1) << "signal_concentration" << sc.signal_concentration;
   DbgLv(1)  << "vbar20" << sc.vbar20;
   DbgLv(1)  << "mw" << sc.mw;
   DbgLv(1) << "s" << sc.s;
   DbgLv(1) << "D" << sc.D;
   DbgLv(1) << "f" << sc.f;
   DbgLv(1) << "f_f0" << sc.f_f0;
   DbgLv(1) << "extinction" << sc.extinction;
   DbgLv(1) << "axial_ratio" << sc.axial_ratio;
   DbgLv(1) << "sigma" << sc.sigma;
   DbgLv(1) << "delta" << sc.delta;
   DbgLv(1)  << "oligomer" << sc.oligomer;
   DbgLv(1)  << "shape" << sc.shape;
   DbgLv(1)  << "name" << sc.name;
   DbgLv(1)  << "analyte_type" << sc.analyte_type;
   // DbgLv(1) << "mfem_initial:";
   DbgLv(1) << "----------------Simulation component parameters ends here---------------";
 //   dump_mfem_initial( sc.c0 );
}

// Prints simparams structure
void US_Astfem_RSA::printsimparams( void )
{
   DbgLv(1) << "speed profile list size " << simparams.speed_step.size();
   DbgLv(1) << "simpoints" << simparams.simpoints;
   DbgLv(1) << "meshType " << simparams.meshType;
   DbgLv(1) << "gridType " << simparams.gridType;
   DbgLv(1) << "radial_resolution " << simparams.radial_resolution;
   DbgLv(1) << "meniscus " << simparams.meniscus;
   DbgLv(1) << "bottom " << simparams.bottom;
   DbgLv(1) << "rnoise " << simparams.rnoise;
   DbgLv(1) << "lrnoise " << simparams.lrnoise;
   DbgLv(1) << "tinoise " << simparams.tinoise;
   DbgLv(1) << "rinoise " << simparams.rinoise;
   DbgLv(1) << "rotorCalID " << simparams.rotorCalID;
   DbgLv(1) << "band_forming " << simparams.band_forming;
   DbgLv(1) << "band_volume " << simparams.band_volume;
   DbgLv(1) << "firstScanIsConcentration "<< simparams.firstScanIsConcentration;
}

// Returns a string with curent date_hr+minute+seconds
QString US_Astfem_RSA::temp_Id_name()
{
   return  ( "p" + QString::number( getpid() ) +
             "t" + QDateTime::currentDateTime().toUTC()
                   .toString( "yyMMddhhmmss" ) );
}

// Sets the buffer
void US_Astfem_RSA::set_buffer( US_Buffer buffer )
{
   density     = buffer.density;             // for compressibility
   compressib  = buffer.compressibility;

   buffer.compositeCoeffs( d_coeff, v_coeff );
}

bool US_Astfem_RSA::iszero( const double s, const double D,        const double rpm,
                            const double t, const double meniscus, const double bottom )
{
   double tolerance      = 0.001;
   double sqr_D          = sqrt( D );
   double sqr_t          = ( t > 0.0 ) ? sqrt( t ) : 1.0;
   double rpm_u          = ( rpm > 0.0 ) ? rpm     : 400.0;
   double omega_sq       = sq( rpm_u * M_PI / 30.0 );
   double s_distance     = meniscus * exp( s * omega_sq * t );
   double back_diffusion = tolerance * sqr_D
                           / ( s * omega_sq * ( bottom + meniscus ) * sqr_t );
   double xval           = US_Math2::find_root( back_diffusion );
   double radD           = bottom - ( 2.0 * xval * sqr_D * sqr_t );
   double cleared        =  s_distance - bottom + radD;

   // Make sure the final value is within the possible range
   //   radD = qMax( meniscus+0.015, qMin( bottom, radD ) )
   bool is_zero          = ( cleared > bottom );
DbgLv(1) << "IS_Z: radD" << radD << "s_dist" << s_distance
 << "back_diffus" << back_diffusion << "xval" << xval << "cleared" << cleared
 << "is_zero" << is_zero;
DbgLv(1) << "IS_Z:    s D rpm t men bot" << s << D << rpm << t << meniscus << bottom
 << "   omega_sq  sqr_t" << omega_sq << sqr_t;

   return is_zero;
}

