//! \file us_astfem_rsa.h

#include "us_astfem_rsa.h"
#include "us_astfem_math.h"
#include "us_femglobal.h"
#include "us_hardware.h"
#include "us_math.h"
#include "us_stiffbase.h"
#include "us_sleep.h"

US_Astfem_RSA::US_Astfem_RSA( struct ModelSystem&          model, 
                              struct SimulationParameters& params, 
                              QObject*                     parent ) 
   : QObject( parent ), system( model ), simparams( params )
{
   stopFlag        = false;
   use_time        = false;
   time_correction = true;
   show_movie      = false;
}

int US_Astfem_RSA::calculate( //struct ModelSystem&          system, 
                              //struct SimulationParameters& simparams,
                              QList< struct mfem_data >&   exp_data ) 
{
   mfem_initial* vC0 = NULL;    // Initial concentration for multiple components
   mfem_initial  CT0;           // Initial total concentration
   mfem_data     simdata;
   float         current_time;
   double        current_speed;

   int           duration;
   int           initial_npts = 1000;
   int           current_assoc;
   int           size_cv         = system.component_vector.size();
   bool*         reacting        = new bool[ size_cv ];

   double        accel_time;
   double        dr;
   
   af_params.first_speed = simparams.speed_step[ 0 ].rotorspeed;
   af_params.simpoints   = simparams.simpoints;
   
   update_assocv();
   initialize_rg();  // Reaction group
   adjust_limits( simparams.speed_step[ 0 ].rotorspeed );

   for ( int k = 0; k < size_cv; k++ )
   {
      struct SimulationComponent* sc = &system.component_vector[ k ];
      reacting[ k ] = false;

      for ( int j = 0; j <  system.assoc_vector.size(); j++ )
      {
         for ( int n = 0; n < system.assoc_vector[ j ].comp.size(); n++ )
         {
            if ( k == (int) system.assoc_vector[ j ].comp[ n ] )
            {
                reacting[ k ] = true;
                current_assoc = j;
                break;   // Since a comp appears at most once in an assoc rule
            }
         }
      }

      current_time  = 0.0;
      last_time     = 0.0;
      current_speed = 0.0;
      w2t_integral  = 0.0;
      
      CT0.radius.clear();
      CT0.concentration.clear();
      
      dr = ( af_params.current_bottom - af_params.current_meniscus ) / 
           ( initial_npts - 1 );
      
      for ( int j = 0; j < initial_npts; j++ )
      {
         CT0.radius << af_params.current_meniscus + j * dr;
         CT0.concentration << 0.0;
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
      
      if ( simparams.band_firstScanIsConcentration )
      {
         mfem_initial scan1;
         scan1.radius.clear();
         scan1.concentration.clear();

         for ( int j = 0; j < exp_data[ 0 ].scan[ 0 ].conc.size(); j++ )
         {
            scan1.radius << exp_data[ 0 ].radius[ j ];
            scan1.concentration << exp_data[ 0 ].scan[ 0 ].conc[ j ];
         }

         US_AstfemMath::interpolate_C0( scan1, sc->c0 );
      }

      if ( ! reacting[ k ] ) // noninteracting
      {
         initialize_conc( k, CT0, true );

         af_params.s.clear();  
         af_params.D.clear();
         af_params.kext.clear();

         af_params.s    << sc->s;
         af_params.D    << sc->D;
         af_params.kext << sc->extinction;
         
         for ( int step = 0; step < simparams.speed_step.size(); step++ )
         {
            struct SpeedProfile* sp = &simparams.speed_step[ step ]; 
            struct mfem_data*    ed = &exp_data            [ step ]; 
            adjust_limits( sp->rotorspeed );

            ed->meniscus = af_params.current_meniscus;
            ed->bottom   = af_params.current_bottom;
            accel_time   = 0.0;

            // We need to simulate acceleration
            if ( sp->acceleration_flag ) 
            {
               // If the speed difference is larger than acceleration rate then
               // we have at least 1 acceleration step
               
               af_params.time_steps = (uint) 
                  fabs( sp->rotorspeed - current_speed ) / sp->acceleration;
               
               // Each simulation step is 1 second in the acceleration phase
               af_params.dt = 1.0;
               af_params.simpoints = 2 * simparams.simpoints; 
               
               // Use a fixed grid with refinement at both ends and with 
               // twice the number of points
               af_params.start_time = current_time;
               
               calculate_ni( current_speed, sp->rotorspeed, CT0, simdata, true );

               // Add the acceleration time:
               accel_time    = af_params.dt * af_params.time_steps;
               current_time += accel_time;

               emit new_time( current_time );
               qApp->processEvents();
               
               if ( stopFlag ) return 1;
            }  // End of acceleration

            duration = (uint) ( sp->duration_hours * 3600 + 
                                sp->duration_minutes * 60 );

            if ( step == simparams.speed_step.size() - 1 )
               duration += (uint) ( duration * 0.05 ); // + 5% 

            if ( accel_time > duration )
            {
               qDebug() << "Attention: acceleration time exceeds duration - "
                           "please check initialization\n";
               return -1;
            }
            else
            {
               duration -= (uint) accel_time;
            }

            double omega = sp->rotorspeed * M_PI / 30;
            af_params.omega_s = sq( omega );
            
            af_params.dt = log( ed->bottom / ( ed->meniscus ) )
                        / ( ( fabs( sc->s ) * af_params.omega_s ) 
                              * ( simparams.simpoints - 1 ) );
            
            if ( af_params.dt > duration)
            {
               af_params.dt        = duration;
               af_params.simpoints = 1 + 
                  (uint) ( log( ed->bottom / ed->meniscus ) /
                           ( fabs( sc->s ) * af_params.omega_s * af_params.dt ) 
                         );
            }

            if ( af_params.simpoints > 10000 ) af_params.simpoints = 10000;

            // Find out the minimum number of simpoints needed to provide 
            // the necessary dt:
            af_params.time_steps = (uint) ( 1 + duration / af_params.dt );
            af_params.start_time = current_time;
            
            calculate_ni( sp->rotorspeed, sp->rotorspeed, CT0, simdata, false );

            // Set the current time to the last scan of this speed step
            current_time = sp->duration_hours * 3600 +
                           sp->duration_minutes * 60;
            
            // Interpolate the simulated data onto the experimental time and 
            // radius grid
            US_AstfemMath::interpolate( *ed, simdata, use_time );
            
            // Set the current speed to the constant rotor speed of the 
            // current speed step
            current_speed = sp->rotorspeed;

            qApp->processEvents();

            if ( stopFlag ) return 1;
         } // Speed step loop
         
         emit current_component( k + 1 );
         qApp->processEvents();
      }
   }
   
   // Resize af_params.local_index
   af_params.local_index.clear();
   for ( int i = 0; i < size_cv; i++ ) af_params.local_index << 0;
   
   struct ComponentRole cr;

   for ( int group = 0; group < rg.size(); group++ )
   {
      uint num_comp = rg[ group ].GroupComponent.size();
      uint num_rule = rg[ group ].association.size();
      af_params.rg_index = group;
      
      af_params.s.clear();
      af_params.D.clear();
      af_params.kext.clear();
      af_params.role.clear();
      af_params.association.clear();

      for ( uint m = 0; m < num_rule; m++ )
         af_params.association << system.assoc_vector[ rg[ group ].association[ m ] ];

      for ( uint j = 0; j < num_comp; j++ )
      {
         int                         index = rg[ group ].GroupComponent[ j ];
         struct SimulationComponent* sc    = &system.component_vector[ index ];
         af_params.s    << sc->s;
         af_params.D    << sc->D;
         af_params.kext << sc->extinction;
         
         // Global to local index
         af_params.local_index[ index ] = j;
         
         af_params.role << cr;  // Add j'th rule      
         af_params.role[ j ].comp_index = index;
         af_params.role[ j ].assoc.clear();
         af_params.role[ j ].react.clear();
         af_params.role[ j ].st.clear();

         // Check all assoc rule in this rg
         for ( int m = 0; m < rg[ group ].association.size(); m++ ) 
         {
            // Check all comp in rule
            uint                rule = rg[ group ].association[ m ];
            struct Association* as   = &system.assoc_vector[ rule ];

            for ( int n = 0; n < as->comp.size(); n++ )
            {
               if ( af_params.role[ j ].comp_index == as->comp[ n ] )
               {
                  af_params.role[ j ].assoc << m ;  // local index for the rule
                  af_params.role[ j ].react << as->react [ n ];
                  af_params.role[ j ].st    << as->stoich[ n ];
                  break;
               }
            }
         }
      }

      for ( uint m = 0; m < num_rule; m++ )
      {
          for ( int n = 0; n < af_params.association[ m ].comp.size(); n++ )
          {
            af_params.association[ m ].comp[ n ] = 
               af_params.local_index[ af_params.association[ m ].comp[ n ] ];
          }
      }

      current_time  = 0.0;
      current_speed = 0.0;
      w2t_integral  = 0.0;
      last_time     = 0.0;

      dr = ( af_params.current_bottom - af_params.current_meniscus ) /
           ( initial_npts - 1 );
      
      if ( vC0 != NULL ) delete [] vC0;
      
      vC0 = new mfem_initial[ rg[ group ].GroupComponent.size() ];
      
      for ( int j = 0; j < rg[ group ].GroupComponent.size(); j++ )
      {
         CT0.radius.clear();
         CT0.concentration.clear();

         for ( int i = 0; i < initial_npts; i++ )
         {
            CT0.radius        << af_params.current_meniscus + i * dr;
            CT0.concentration << 0.0;
         }

         initialize_conc( rg[ group ].GroupComponent[ j ], CT0, false );
         vC0[ j ] = CT0;
      }

      decompose( vC0 );

      for ( int ss = 0; ss < simparams.speed_step.size(); ss++ )
      {
         struct SpeedProfile* sp = &simparams.speed_step[ ss ];
         struct mfem_data*    ed = &exp_data            [ ss ]; // exp data

         adjust_limits( sp->rotorspeed );
         ed->meniscus = af_params.current_meniscus;
         ed->bottom   = af_params.current_bottom;
         accel_time              = 0.0;
         
         // We need to simulate acceleration
         if ( sp->acceleration_flag )
         {
            // If the speed difference is larger than acceleration 
            // rate then we have at least 1 acceleration step
            
            af_params.time_steps = (uint) 
               ( fabs( sp->rotorspeed - current_speed ) / sp->acceleration );
            
            // Each simulation step is 1 second long in the acceleration phase
            af_params.dt        = 1.0;
            af_params.simpoints = 2 * simparams.simpoints; 
            
            // Use a fixed grid with refinement at both ends and with twice 
            // the number of points
            af_params.start_time = current_time;
            
            calculate_ra2( current_speed, (double) sp->rotorspeed, 
                  vC0, simdata, true ); 
            
            // Add the acceleration time:
            accel_time    = af_params.dt * af_params.time_steps;
            current_time += accel_time;
 
            emit new_time( current_time );
            qApp->processEvents();

            if ( stopFlag ) return 1;
         }  // End of for acceleration

         duration = (uint) 
            ( sp->duration_hours * 3600 + sp->duration_minutes * 60 );
         
         if ( ss == simparams.speed_step.size() - 1 )
            duration += (uint) ( duration * 0.05 ); // + 5% 
         
         if ( accel_time > duration )
         {
            qDebug() << "Attention: acceleration time exceeds duration - "
                        "please check initialization\n";
            return -1;
         }
         else
         {
            duration -= (unsigned int) accel_time;
         }

         double s_max = fabs( af_params.s[ 0 ] );     // Find the largest s
         
         for ( int m = 1; m < af_params.s.size(); m++ )
             if ( s_max < fabs( af_params.s[m] ) ) s_max = fabs( af_params.s[m] );


         af_params.omega_s = sq( sp->rotorspeed * M_PI / 30 );
         
         af_params.dt = log( ed->bottom / ed->meniscus) /
                    ( af_params.omega_s * s_max * ( simparams.simpoints - 1 ) );

         if (af_params.dt > duration )
         {
            af_params.dt = duration;
            af_params.simpoints = 
               1 + (uint) ( log( ed->bottom / ed->meniscus )
               / ( s_max * af_params.omega_s * af_params.dt ) );
         }

         if ( af_params.simpoints > 10000 ) af_params.simpoints = 10000;

         // Find out the minimum number of simpoints needed to provide the 
         // necessary dt:
         af_params.time_steps = (unsigned int) ( 1 + duration / af_params.dt );

         af_params.start_time = current_time;
         calculate_ra2( (double) sp->rotorspeed, (double) sp->rotorspeed, 
               vC0, simdata, false );
         
         // Set the current time to the last scan of this speed step
         current_time = sp->duration_hours * 3600 + sp->duration_minutes * 60;
         
         // Interpolate the simulated data onto the experimental 
         // time and radius grid
         US_AstfemMath::interpolate( *ed, simdata, use_time );
         
         // Set the current speed to the constant rotor speed of the 
         // current speed step
         current_speed = sp->rotorspeed;
         
         qApp->processEvents();
         
         if ( stopFlag ) return 1 ; 
      } // Speed step loop
   }

   // k is out of scope here!!!!!!!
   //emit current_component( k + 1 );
   qApp->processEvents();

   if ( time_correction )
   {
      // Check each speed step to see if it contains acceleration
      for ( int ss = 0; ss < simparams.speed_step.size(); ss++ ) 
      {
         struct SpeedProfile* sp = &simparams.speed_step[ ss ];
         struct mfem_data*    ed = &exp_data            [ ss ]; // exp data
         
         // We need to correct time
         if ( simparams.speed_step[ ss ].acceleration_flag ) 
         {
            float slope;
            float intercept;
            float correlation; 
            float sigma;
            float correction;
            
            float* xtmp = new float [ sp->scans ];
            float* ytmp = new float [ sp->scans ];

            // Only fit the scans that belong to this speed step
            for ( uint i = 0; i < simparams.speed_step[ ss ].scans; i++ ) 
            {
               xtmp[ i ] = ed->scan[ i ].time;
               ytmp[ i ] = ed->scan[ i ].omega_s_t;
            }

            US_Math::linefit( &xtmp, &ytmp, &slope, &intercept, &sigma, 
                              &correlation, sp->scans );

            correction = -intercept / slope;
            
            for ( uint i = 0; i < sp->scans; i++ )
               ed->scan[ i ].time -= correction;
            
            delete [] xtmp;
            delete [] ytmp;
         }
      }
   }

   if ( vC0 != NULL ) delete [] vC0;
   delete [] reacting;
 
   return 0;
}


void US_Astfem_RSA::update_assocv( void )
{
   for ( int i = 0; i < system.assoc_vector.size(); i++ )
   {
      struct Association* as = &system.assoc_vector[ i ];
      
      as->comp  .clear();
      as->stoich.clear();
      as->react .clear();
      
      if ( as->component3 == -1 )
      {
         as->comp << as->component1;
         as->comp << as->component2;
         
         as->stoich << as->stoichiometry2;
         as->stoich << as->stoichiometry1;
         
         as->react << 1;
         as->react << -1;
      }
      else
      {
         as->comp << as->component1;
         as->comp << as->component2;
         as->comp << as->component3;
         
         as->stoich << as->stoichiometry1;
         as->stoich << as->stoichiometry2;
         as->stoich << as->stoichiometry3;
         
         as->react << 1;
         as->react << 1;
         as->react << -1;
      }
   }
}

void US_Astfem_RSA::adjust_limits( uint speed )
{
   // First correct meniscus to theoretical position at rest:
   double stretch_value = stretch( simparams.rotor, af_params.first_speed );
  
   // This is the meniscus at rest
   af_params.current_meniscus = simparams.meniscus - stretch_value;
 
   // Calculate rotor stretch at current speed
   stretch_value = stretch( simparams.rotor, speed );

   // Add current stretch to meniscus at rest
   af_params.current_meniscus += stretch_value;
   
   // Add current stretch to bottom at rest
   af_params.current_bottom = simparams.bottom + stretch_value;
}

double US_Astfem_RSA::stretch( int rotor, uint rpm )
{
   QList< struct rotorInfo > rotor_list;
   rotor_list.clear();
   
   double stretch = 0.0;
   US_Hardware::readRotorInfo( rotor_list );
                  
   for ( int i = 0; i < 5; i++ )
   {
      stretch += rotor_list[ rotor ].coefficient[ i ] * 
         pow( (double) rpm, (double) i );
   }
   
   return stretch;
}

// Setup reaction groups
void US_Astfem_RSA::initialize_rg( void )
{
   rg.clear();

   // If there are no reactions, then it is all noninteracting
   if ( system.assoc_vector.size() == 0 ) return; 
   
   QList< bool > reaction_used;
   reaction_used.clear();

   for ( int i = 0; i < system.assoc_vector.size(); i++ )
      reaction_used << false;
   
   // Initialize the first reaction group and put it into a temporary reaction
   // group, use as test against all assoc vector entries:
   
   struct ReactionGroup tmp_rg;
   tmp_rg.GroupComponent.clear();
   tmp_rg.association.clear();
   
   tmp_rg.association << 0;
   tmp_rg.GroupComponent << system.assoc_vector[ 0 ].component1;
   tmp_rg.GroupComponent << system.assoc_vector[ 0 ].component2;

   // Only 2 components react in first reaction
   if ( system.assoc_vector[ 0 ].component3  != -1 )
      tmp_rg.GroupComponent <<  system.assoc_vector[0].component3;
   
   reaction_used[ 0 ] = true;

   // There is only one reaction, so add it and return
   if ( system.assoc_vector.size() == 1 )
   {
      rg << tmp_rg;
      return;
   }

   bool flag3 = false;
   
   for ( int i = 0; i < system.assoc_vector.size(); i++ )
   {
      // Check each association rule to see if it contains components that
      // match tmp_rg components
      
      for ( int counter = 1; counter < system.assoc_vector.size(); counter++ ) 
      {
         struct Association* av = &system.assoc_vector[counter];

         while ( reaction_used[ counter ] )
         {
            counter++;
            if ( counter == system.assoc_vector.size() ) return;
         }
   
         // Check if any component already present in tmp_rg matches any of the
         // three components in current (*system).assoc_vector entry

         bool flag1;
         
         for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ ) 
         {
            flag1 = false;

            if ( av->component1 == (int) tmp_rg.GroupComponent[ j ]  ||
                 av->component2 == (int) tmp_rg.GroupComponent[ j ]  ||
                 av->component3 == (int) tmp_rg.GroupComponent[ j ] )
            {
               flag1 = true;
               break;
            }
         }
        
         // If the component from tmp_rg is present in another
         // system.assoc_vector entry, find out if the component from
         // system.assoc_vector is already in tmp_rg.GroupComponent

         if ( flag1 )  
         {       
            // It is present (flag1=true) so add this rule to the tmp_rg vector
            tmp_rg.association << counter; 
            reaction_used[ counter ] = true;
            
            // There is at least one of all system.assoc_vector entries in
            // this reaction_group, so set flag3 to true
            
            flag3 = true; 
            bool flag2 = false;
            
            // Check if 1st component is already in the GroupVector from tmp_rg
            for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ ) 
            {
               if ( av->component1 == (int) tmp_rg.GroupComponent[ j ])
                  flag2 = true;
            }

            // Add if not present already
            if ( ! flag2 ) tmp_rg.GroupComponent << av->component1;

            flag2 = false;
            
            // Check if 2nd component is already in the GroupVector from tmp_rg
            for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ ) 
            {
               if ( av->component2 == (int) tmp_rg.GroupComponent[ j ]) 
                  flag2 = true; 
            }

            // Add if not present already
            if ( ! flag2 )  tmp_rg.GroupComponent << av->component2; 

            flag2 = false;

            // Check if 3rd component is already in the GroupVector from tmp_rg
            // (but only if non-zero)
            
            if ( av->component3 != -1 ) 
            {
               for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ )
               {
                  if ( av->component3 == (int) tmp_rg.GroupComponent[ j ] )
                     flag2 = true;
               }
               
               // Add if not present already
               if ( ! flag2 ) tmp_rg.GroupComponent <<  av->component3; 
            }
         } 
      }

      if ( flag3 )
      {
         flag3 = false;
         rg << tmp_rg;
         
         tmp_rg.GroupComponent.clear();
         tmp_rg.association.clear();

         // Make the next unused reaction the test reaction
         int j = 1;
         
         while ( reaction_used[ j ] ) j++;

         struct Association* avj = &system.assoc_vector[ j ];
         
         if ( j < system.assoc_vector.size() )
         {
            tmp_rg.association    << j;
            tmp_rg.GroupComponent << avj->component1;
            tmp_rg.GroupComponent << avj->component2;
            
            // Only 2 components react in first reaction
            if ( avj->component3 != -1 )
               tmp_rg.GroupComponent << avj->component3;

            reaction_used[ j ] = true;
            //counter++;   // Out of scope!!!!
         }

         if ( j == system.assoc_vector.size() - 1 )
         {
            rg << tmp_rg;
            return;
         }
      }
   }
}

// Initializes total concentration vector
void US_Astfem_RSA::initialize_conc( uint k, struct mfem_initial& CT0, 
      bool noninteracting ) 
{
   struct SimulationComponent* sc = &system.component_vector[ k ];

   // We don't have an existing CT0 concentration vector. Build up the initial
   // concentration vector with constant concentration
 
   if ( sc->c0.concentration.size() == 0 ) 
   {
      if ( simparams.band_forming )
      {
         // Calculate the width of the lamella
         double base = af_params.current_meniscus * af_params.current_meniscus 
            + simparams.band_volume * 360.0 / ( 2.5 * 1.2 * M_PI );

         double lamella_width = pow( base, 0.5 ) - af_params.current_meniscus;
            
         // Calculate the spread of the lamella:
         for ( int j = 0; j < CT0.concentration.size(); j++ )
         {
            base = ( CT0.radius[ j ] - af_params.current_meniscus) / lamella_width;
            
            CT0.concentration[ j ] += 
               sc->concentration * exp( -pow( base, 4.0 ) );
         }
      }
      else
      {
         for ( int j = 0; j < CT0.concentration.size(); j++ )
            CT0.concentration[j] += sc->concentration;
      }
   }
   else
   {
      if ( noninteracting )
      {
         // Take the existing initial concentration vector and copy it to the
         // temporary CT0 vector: needs rubber band to make sure meniscus and
         // bottom equal current_meniscus and current_bottom
         
         CT0.radius.clear();
         CT0.concentration.clear();
         CT0 = system.component_vector[ k ].c0;
      }
      else // interpolation
      {
         mfem_initial C;
         C.radius.clear();
         C.concentration.clear();
         
         double dr = (af_params.current_bottom - af_params.current_meniscus) /
                     ( CT0.concentration.size() - 1 );
         
         for ( int j = 0; j < CT0.concentration.size(); j++ )
         {
            C.radius        << af_params.current_meniscus + j * dr;
            C.concentration << 0.0;
         }

         US_AstfemMath::interpolate_C0 ( sc->c0, C );
         
         for ( int j = 0; j < CT0.concentration.size(); j++ )
            CT0.concentration[ j ] += C.concentration[ j ];
      }
   }
}

// Non-interacting solute, constant speed
int US_Astfem_RSA::calculate_ni( double rpm_start, double rpm_stop, 
      mfem_initial& C_init, mfem_data& simdata, bool accel )
{
   double** CA = NULL;     // stiffness matrix on left hand side
                           // CA[0...Ms-1][0...N-1][4]
   
   double** CB = NULL;     // stiffness matrix on right hand side
                           // CB[0...Ms-1][0...N-1][4]
   
   double*  C0 = NULL;     // C[m][j]: current/next concentration of m-th component at x_j
   double*  C1 = NULL;     // C[0...Ms-1][0....N-1]:
   
   double** CA1;           // for matrices used in acceleration
   double** CA2;
   double** CB1;
   double** CB2;

   simdata.radius.clear();
   simdata.scan.clear();
   mfem_scan simscan;

   // Generate the adaptive mesh
  
   double sw2 = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30 );
   QList< double > nu;
   nu.clear();
   nu << sw2 / af_params.D[ 0 ];
   
   mesh_gen( nu, simparams.mesh );
   
   // Refine left hand side (when s>0) or right hand side (when s < 0) for
   // acceleration

   if ( accel )     
   {                             
      uint   j;
      double xc;
      
      if ( af_params.s[ 0 ] > 0 )
      { 
         // Radial distance from meniscus how far the boundary will move during
         // this acceleration step (without diffusion)
         
         xc = af_params.current_meniscus +  
              sw2 * ( af_params.time_steps * af_params.dt ) / 3.0;
         
         for ( j = 0; j < N - 3; j++ )
            if ( x[ j ] > xc ) break; 
      }
      else
      {
         xc = af_params.current_bottom +  
              sw2 * ( af_params.time_steps * af_params.dt ) / 3.0;

         for ( j = 0; j < N - 3; j++ )
            if ( x[ N - j - 1 ] < xc ) break;
      }

      mesh_gen_RefL( j + 1, 4 * j );
   }

   for ( uint i = 0; i < N; i++ ) simdata.radius << x[ i ];

   // Initialize the coefficient matrices

   US_AstfemMath::initialize_2d( 3, N, &CA );
   US_AstfemMath::initialize_2d( 3, N, &CB );

   if ( ! accel ) // No acceleration
   {
      sw2 = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30 );
      
      if ( ! simparams.moving_grid )
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
      US_AstfemMath::initialize_2d( 3, N, &CA1 );
      US_AstfemMath::initialize_2d( 3, N, &CA2 );
      US_AstfemMath::initialize_2d( 3, N, &CB1 );
      US_AstfemMath::initialize_2d( 3, N, &CB2 );

      sw2 = 0.0;
      ComputeCoefMatrixFixedMesh( af_params.D[ 0 ], sw2, CA1, CB1 );

      sw2 = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30 );
      ComputeCoefMatrixFixedMesh( af_params.D[ 0 ], sw2, CA2, CB2 );
   }

   // Initial condition
   C0 = new double [ N ];
   C1 = new double [ N ];

   // Interpolate the given C_init vector on the new C0 grid
   US_AstfemMath::interpolate_C0( C_init, C0, x );

   // Time evolution
   double* right_hand_side = new double [ N ];

   // Calculate all time steps 
   for ( uint i = 0; i < af_params.time_steps + 1; i++ )
   {
      double rpm_current = rpm_start + 
         ( rpm_stop - rpm_start ) * ( i + 0.5 ) / af_params.time_steps;
      
      emit current_speed( (uint) rpm_current );

      if ( accel ) // Then we have acceleration
      {
         for ( uint j1 = 0; j1 < 3; j1++ )
         {
            for ( uint j2 = 0; j2 < N; j2++ )
            {
               CA[ j1 ][ j2 ] = CA1[ j1 ][ j2 ] + sq( rpm_current / rpm_stop ) * 
                  ( CA2[ j1 ][ j2 ] - CA1[ j1 ][ j2 ] );
               
               CB[ j1 ][ j2 ] = CB1[ j1 ][ j2 ] + sq( rpm_current / rpm_stop ) * 
                  ( CB2[ j1 ][ j2 ] - CB1[ j1 ][ j2 ] );
            }
         }
      }

      simscan.rpm  = (uint) rpm_current;
      simscan.time = af_params.start_time + i * af_params.dt;
      
      w2t_integral += ( simscan.time - last_time ) * 
                      sq( rpm_current * M_PI / 30 );
      
      last_time         = simscan.time;
      simscan.omega_s_t = w2t_integral;

      if ( show_movie )
      {
         emit new_scan( x, C0 );
         emit new_time( (float) simscan.time );
         qApp->processEvents();
         US_Sleep::msleep( 10 ); // 10 ms to let the display update.
      }
     
      simscan.conc.clear();

      for ( uint j = 0; j < N; j++ ) simscan.conc << C0[ j ];

      simdata.scan << simscan;

      // Sedimentation part:
      // Calculate the right hand side vector 
      
      if ( accel || ! simparams.moving_grid )
      {
         right_hand_side[ 0 ] = - CB[ 1 ][ 0 ] * C0[ 0 ] 
                                - CB[ 2 ][ 0 ] * C0[ 1 ];

         for ( uint j = 1; j < N - 1; j++ )
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
            
            for ( uint j = 2; j < N; j++ )
            {
               right_hand_side[ j ] = - CB[ 0 ][ j ] * C0[ j - 2 ] 
                                      - CB[ 1 ][ j ] * C0[ j - 1 ] 
                                      - CB[ 2 ][ j ] * C0[ j     ];
            }
         }
         else
         {
            for ( uint j = 0; j < N - 2; j++ )
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

      US_AstfemMath::tridiag( CA[0], CA[1], CA[2], right_hand_side, C1, N );
      
      for ( uint j = 0; j < N; j++ ) C0[ j ] = C1[ j ];
   } // time loop

   C_init.radius.clear();
   C_init.concentration.clear();
   
   for ( uint j = 0; j < N; j++ )
   {
      C_init.radius        <<  x [ j ];
      C_init.concentration <<  C1[ j ];
   }

   delete [] right_hand_side;
   delete [] C0;
   delete [] C1;
   
   US_AstfemMath::clear_2d( 3, CA );
   US_AstfemMath::clear_2d( 3, CB );
   
   if ( accel )
   {
      US_AstfemMath::clear_2d( 3, CA1 );
      US_AstfemMath::clear_2d( 3, CB1 );
      US_AstfemMath::clear_2d( 3, CA2 );
      US_AstfemMath::clear_2d( 3, CB2 );
   }

   return 0;
}

void US_Astfem_RSA::mesh_gen( QList< double >& nu, uint MeshOpt )
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
//              = 2 Exponential mesh (Schuck's formular, no refinement at bottom)
//              = 3 input from data file: "mesh_data.dat"
//              = 10, acceleration mesh (left and right refinement)
//////////////////////////////////////////////////////////////%

////////////////////%
// generate the mesh
////////////////////%

   x.clear();

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   uint   NN = af_params.simpoints;

   
   switch ( MeshOpt )
   {
      //////////////////////%
      // Mesh Type 0 (default): adaptive mesh based on all nu
      //////////////////////%

      case 0: // Astfem without left hand refinement
         qSort( nu.begin(), nu.end() );   // put nu in ascending order

         if ( nu[ 0 ] > 0 )
            mesh_gen_s_pos( nu );

         else if ( nu[ nu.size() - 1 ] < 0 )
            mesh_gen_s_neg( nu );
         
         else       // Some species with s < 0 and some with s > 0
         {
            for ( uint i = 0; i < NN; i++ ) x << m + ( b - m ) * i / ( NN - 1 );
         }
         break;
      
      case 1: // Claverie mesh without left hand refinement

         for ( uint i = 0; i < NN; i++ )  x <<  m + ( b - m ) * i / ( NN - 1 );
         break;

      case 2: // Moving Hat (Peter Schuck's Mesh) w/o left hand side refinement
         
         x << m;
         
         // Standard Schuck grids
         for ( uint i = 1; i < NN - 1; i++ )
            x <<  m * pow( b / m, ( i - 0.5 ) / ( NN - 1 ) );
         
         x << b;
         break;

      case 3: // User defined mesh generated from data file
         { 
            QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );

            QFile f( home + "/etc/mesh.dat" );

            if ( f.open( QIODevice::ReadOnly ) )
            {
               QTextStream ts( &f );
               
               while ( ! ts.atEnd() )  x << ts.readLine().toDouble(); 
               
               f.close();

               if ( fabs( x[ 0 ] - m ) > 1.0e7 )
               {
                  qDebug() << "The meniscus from the mesh file does not match "
                              "the set meniscus - using Claverie Mesh instead\n";
               }

               if ( fabs( x[ x.size() - 1 ] - b ) > 1.0e7 )
               {
                  qDebug() << "The cell bottom from the mesh file does not match "
                              "the set cell bottom - using Claverie Mesh instead\n";
               }
            }
            else
            {
               qDebug() << "Could not read the mesh file - "
                           "using Claverie Mesh instead\n";
               
               for ( uint i = 0; i < af_params.simpoints; i++ )
                  x <<  m + ( b - m ) * i / ( NN - 1 );
            }
            break;
         }
      
      default:
         qDebug() << "undefined mesh option\n";
         break;

   }

   N = x.size();
}

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell bottom (for s>0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_pos( const QList< double >& nu )
{
   double          tmp_Hstar;
   QList< double > xc;
   QList< double > Hstar;
   QList< double > y;
   QList< uint >   Nf;
   
   xc.clear();
   Hstar.clear();
   Nf.clear();

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   uint   NN = af_params.simpoints;

   int    IndLayer = 0;         // number of layers for grids in steep region
   double uth      = 1.0 / NN;  // threshold of u for steep region

   for ( int i = 0; i < af_params.s.size(); i++ ) // Markers for steep regions
   {
      double tmp_xc = b - ( 1.0 / ( nu[ i ] * b ) ) * 
         log( nu[ i ] * ( sq( b ) - sq( m ) ) ) / ( 2.0 * uth );
      
      // # of pts for i-th layer
      int tmp_Nf = (int) ( M_PI / 2.0 * ( b - tmp_xc )
            * nu[ i ] * b / 2.0 + 0.5 ) + 1; 
      
      // Step required by Pac(i) < 1
      tmp_Hstar = ( b - tmp_xc ) / tmp_Nf * M_PI / 2.0;
      
      if ( ( tmp_xc > m ) &&
           ( b - m * pow( b / m, ( NN - 2.0 ) /  ( NN - 1.0 ) ) > tmp_Hstar ) )
      {
         xc    << tmp_xc;
         Nf    << tmp_Nf;
         Hstar << tmp_Hstar;
         IndLayer++;
      }
   }

   xc << b;

   if ( IndLayer == 0 )   // Use Schuck's grid only
   {
      x << m;

      // Add one more point to Schuck's grids
      for ( uint i = 1; i < NN - 1 ; i++ ) 
         x << m * pow( b / m, (double) i / ( NN - 1 ) ); // Schuck's mesh

      x << b;
   }
   else  // Need a composite grid
   {
      // Steep region
      uint indp = 0;  // Index for a grid point
      
      for ( int i = 0; i < IndLayer; i++ )  // Consider i-th steep region
      {
         if ( i < IndLayer - 1 )  // Use linear distribution for step size distrib
         {
            double HL = Hstar[ i ];
            double HR = Hstar[ i + 1 ];
            uint Mp = (uint) ( ( xc[ i + 1 ] - xc[ i ] ) * 2.0 / ( HL + HR ) );
            
            if ( Mp > 1 ) 
            {
               double beta  = ( ( HR - HL ) / 2.0 ) * Mp ;
               double alpha = ( xc[ i + 1 ] - xc[ i ] ) - beta;

               for ( uint j = 0; j <= Mp - 1; j++ )
               {
                  double xi = (double) j / (double) Mp;
                  y << xc[ i ] + alpha * xi + beta * pow( xi, 2.0 );
                  indp++;
               }
            }
         }
         else     // Last layer, use sine distribution for grids
         {
            for ( uint j = 0; j <= Nf[ i ] - 1; j++ )
            {
               indp++;
               y << xc[ i ] + ( b - xc[ i ] ) * 
                    sin( j / ( Nf[ i ] - 1.0 ) * M_PI / 2.0 );
               
               if ( y[ indp - 1 ] > xc[ i + 1 ] ) break; 
            }
         }
      }

      int NfTotal = indp;
      
      QList< double > ytmp;
      ytmp.clear();

      // Reverse the order of y
      int j = NfTotal;
      
      do { ytmp << y[ --j ]; } while ( j != 0 );
      
      y.clear();
      y = ytmp;
      
      // Transition region
      // Smallest step size in transit region
      double Hf = y[ NfTotal - 2 ] - y[ NfTotal - 1 ];

      // Number of pts in trans region
      uint Nm = (uint) 
          ( log( b / ( ( NN - 1 ) * Hf ) * log( b / m ) ) / log( 2.0 ) ) + 1;
          
      
      double xa = y[ NfTotal - 1 ] - Hf * ( pow( 2.0, (double) Nm ) - 1.0 );
      
      uint Js = (uint) ( ( NN - 1 ) * log( xa / m ) / log( b / m ) );
      
      // xa is  modified so that y[ NfTotal - Nm ] matches xa exactly
      xa = m * pow( b / m, Js / ( NN - 1.0 ) );
      
      double tmp_xc = y[ NfTotal - 1 ];
      double HL     = xa * ( 1.0 - m / b );
      double HR     = y[ NfTotal - 2 ] - y[ NfTotal - 1 ];
      
      uint Mp = (uint)( ( tmp_xc - xa ) * 2.0 / ( HL + HR ) ) + 1;
      
      if ( Mp > 1 ) 
      {
         double beta  = ( ( HR - HL ) / 2.0 ) * Mp;
         double alpha = ( tmp_xc - xa ) - beta;
         
         for ( int j = Mp - 1; j > 0; j-- )
         {
            double xi = (double) j / Mp;
            y << xa + alpha * xi + beta * pow( xi, 2.0 );
         }
      }

      Nm = Mp;

      // Regular region
      x << m;
      
      for ( uint j = 1; j <= Js; j++ )
         x << m * pow( b / m, j / ( NN - 1.0 ) );

      for ( uint j = 0; j < NfTotal + Nm - 1; j++ )
         x << y[ NfTotal + Nm - j - 2 ];
   }
}

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell meniscus (for  s<0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_neg( const QList< double >& nu )
{
   
   unsigned int j, Js, Nf, Nm;
   double xc, xa, Hstar;
   QList< double > yr, ys, yt;

   x.clear();
   yr.clear();
   ys.clear();
   yt.clear();

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   uint   NN = af_params.simpoints;

   double uth = 1.0 / NN;   // Threshold of u for steep region
   double nu0 = nu[ 0 ];
   
   xc = m + 1.0 /  ( fabs( nu0 ) * m ) * 
                   log( ( sq( b ) - sq( m ) ) * fabs( nu0 ) / ( 2.0 * uth ) );
   
   Nf = 1 + (uint)( ( xc - m ) * fabs( nu0 ) * m * M_PI / 4.0 );
   
   Hstar = ( xc - m ) / Nf * M_PI / 2.0;
   
   Nm = 1 + (uint) 
        ( log( m / ( ( NN - 1.0 ) * Hstar ) * log( b / m ) ) / log( 2.0 ) );
   
   xa = xc + ( pow( 2.0, (double) Nm ) - 1.0 ) * Hstar;
   
   Js = (uint) ( ( NN - 1) * log( b / xa ) / log( b / m ) + 0.5 );


   // All grid points at exponentials
   yr << b;

   // Is there a difference between simparams.meniscus and 
   // af_params.current_meniscus??
   for( j = 1; j < NN; j++ )    // Add one more point to Schuck's grids
      yr << b * pow( simparams.meniscus / b, ( j - 0.5 ) / ( NN - 1.0 ) );
   
   yr << m;
   
   if ( b * ( pow( m / b, ( NN - 3.5 ) / ( NN - 1.0 ) )
            - pow( m / b, ( NN - 2.5 ) / ( NN - 1.0 ) ) ) < Hstar || Nf <= 2 )
   {
      // No need for steep region
      for ( j = 0; j < NN; j++ )
      {
         x << yr[ NN - 1 - j ];
      }
      
      qDebug() << "use exponential grid only!\n" << endl;
      
   }
   else
   {
      // Nf > 2
      for ( j = 1; j < Nf; j++ )
         ys << xc - ( xc - m ) * sin(( j - 1.0 ) / ( Nf - 1.0 ) * M_PI / 2.0 );
      
      ys << m;

      for ( j = 0; j < Nm; j++ )
         yt << xc + ( pow( 2.0, (double) j ) - 1.0 ) * Hstar;

      // set x:
      for ( j = 0; j < Nf; j++ )
         x <<  ys[ Nf - 1 - j ] ;
      
      for ( j = 1; j < Nm; j++ )
         x << yt[ j ];
      
      for ( j = Js + 1; j > 0; j-- )
         x << yr[ j - 1 ];

      // Smooth out
      x[ Nf + Nm     ] = ( x[ Nf + Nm - 1 ] + x[Nf + Nm + 1 ] ) / 2.0;
      x[ Nf + Nm + 1 ] = ( x[ Nf + Nm     ] + x[Nf + Nm + 2 ] ) / 2.0;
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
   QList< double > zz;  // temperary array for adaptive grids
   zz.clear();
   
   if ( US_AstfemMath::minval( af_params.s ) > 0 ) // All species with s>0
   {
      // Refine around the meniscus for acceleration
      for ( int j = 0; j < M0; j++ )
      {
         double tmp = (double) j / M0;
         tmp        = 1.0 - cos( tmp * M_PI / 2 );
         zz << x[ 0 ] * ( 1.0 - tmp ) + x[ N0 ] * tmp;
      }

      for ( int j = N0; j < x.size(); j++ ) 
         zz << x[ j ];

      x.clear();

      for ( int j = 0; j < zz.size(); j++ ) 
         x << zz[ j ];
   }
   else if ( US_AstfemMath::maxval( af_params.s ) < 0 ) //  All species with s<0
   {
      for ( int j = 0; j < x.size() - N0; j++ ) 
         zz << x[ j ];
      
      // Refine around the bottom for acceleration
      for ( int j = 1; j <= M0; j++ )
      {
         double tmp = (double) j / M0;
         tmp        = sin( tmp * M_PI / 2 );
         zz << x[ x.size() - N0 - 1 ] * ( 1.0 - tmp ) + x[ x.size() - 1 ] * tmp;
      }
      
      x.clear();
      
      for ( int j = 0; j < zz.size(); j++ ) 
         x << zz[ j ];
   }
   else                  // Sedimentation and floating mixed up
      qDebug() << "No refinement at ends since sedimentation "
                  "and floating mixed ...\n" ;

   N = x.size();
}

// Compute the coefficient matrices based on fixed mesh

void US_Astfem_RSA::ComputeCoefMatrixFixedMesh( 
      double D, double sw2, double** CA, double** CB )
{
   StiffBase stfb0 ;
   double*** Stif;   // Local stiffness matrix at each element
   
   US_AstfemMath::initialize_3d( N - 1, 4, 4, &Stif );

   double xd[ 4 ][ 2 ];     // coord for verices of quad elem
   
   for ( uint k = 0; k < N - 1; k++ )
   {  // loop for all elem
      xd[ 0 ][ 0 ] = x[ k ];
      xd[ 0 ][ 1 ] = 0.0;
      xd[ 1 ][ 0 ] = x[ k + 1 ];
      xd[ 1 ][ 1 ] = 0.0;
      xd[ 2 ][ 0 ] = x[ k + 1 ];
      xd[ 2 ][ 1 ] = af_params.dt;
      xd[ 3 ][ 0 ] = x[ k ];
      xd[ 3 ][ 1 ] = af_params.dt;
      
      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // Assembly coefficient matrices
   // elem[ 0 ]; i=0
   uint k = 0;
   CA[ 1 ][ k ] = Stif[ k ][ 3 ][ 0 ] + Stif[ k ][ 3 ][ 3 ]; // j=3;
   CA[ 2 ][ k ] = Stif[ k ][ 2 ][ 0 ] + Stif[ k ][ 2 ][ 3 ]; // j=2;
   CB[ 1 ][ k ] = Stif[ k ][ 0 ][ 0 ] + Stif[ k ][ 0 ][ 3 ]; // j=0;
   CB[ 2 ][ k ] = Stif[ k ][ 1 ][ 0 ] + Stif[ k ][ 1 ][ 3 ]; // j=1;

   for ( uint k = 1; k < N - 1; k++ )
   {  // loop for all elem
      // elem k-1: i=1,2
      CA[ 0 ][ k ]  = Stif[ k-1 ][ 3 ][ 1 ] + Stif[ k-1 ][ 3 ][ 2 ];  // j=3;
      CA[ 1 ][ k ]  = Stif[ k-1 ][ 2 ][ 1 ] + Stif[ k-1 ][ 2 ][ 2 ];  // j=2;
      CB[ 0 ][ k ]  = Stif[ k-1 ][ 0 ][ 1 ] + Stif[ k-1 ][ 0 ][ 2 ];  // j=0;
      CB[ 1 ][ k ]  = Stif[ k-1 ][ 1 ][ 1 ] + Stif[ k-1 ][ 1 ][ 2 ];  // j=1;

      // elem k: i=0,3
      CA[ 1 ][ k ] += Stif[ k   ][ 3 ][ 0 ] + Stif[ k   ][ 3 ][ 3 ];  // j=3;
      CA[ 2 ][ k ]  = Stif[ k   ][ 2 ][ 0 ] + Stif[ k   ][ 2 ][ 3 ];  // j=2;
      CB[ 1 ][ k ] += Stif[ k   ][ 0 ][ 0 ] + Stif[ k   ][ 0 ][ 3 ];  // j=0;
      CB[ 2 ][ k ]  = Stif[ k   ][ 1 ][ 0 ] + Stif[ k   ][ 1 ][ 3 ];  // j=1;
   }

   // elem[ N-2 ]; i=1,2
   k = N - 1;
   CA[ 0 ][ k ]  = Stif[ k-1 ][ 3 ][ 1 ] + Stif[ k-1 ][ 3 ][ 2 ];  // j=3;
   CA[ 1 ][ k ]  = Stif[ k-1 ][ 2 ][ 1 ] + Stif[ k-1 ][ 2 ][ 2 ];  // j=2;
   CB[ 0 ][ k ]  = Stif[ k-1 ][ 0 ][ 1 ] + Stif[ k-1 ][ 0 ][ 2 ];  // j=0;
   CB[ 1 ][ k ]  = Stif[ k-1 ][ 1 ][ 1 ] + Stif[ k-1 ][ 1 ][ 2 ];  // j=1;
   
   US_AstfemMath::clear_3d( N - 1, 4, Stif );
}

void US_Astfem_RSA::ComputeCoefMatrixMovingMeshR(
      double D, double sw2, double** CA, double** CB )
{
   // Compute local stiffness matrices
   StiffBase stfb0;
   double*** Stif;         // Local stiffness matrix at each element
   double    xd[4][2];     // coord for verices of quad elem
   
   US_AstfemMath::initialize_3d( N, 4, 4, &Stif );

   // elem[0]: triangle
   xd[0][0] = x[0];  xd[0][1] = 0.;
   xd[1][0] = x[1];  xd[1][1] = af_params.dt;
   xd[2][0] = x[0];  xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ 0 ] );

   // elem[k]: k=1..(N-2), quadrilateral
   for ( uint k = 1; k < N - 1; k++ ) // loop for all elem
   {  
      xd[0][0] = x[k-1];   xd[0][1] = 0.0;
      xd[1][0] = x[k  ];   xd[1][1] = 0.0;
      xd[2][0] = x[k+1];   xd[2][1] = af_params.dt;
      xd[3][0] = x[k  ];   xd[3][1] = af_params.dt;
      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // elem[N-1]: triangle
   xd[0][0] = x[N-2];   xd[0][1] = 0.0;
   xd[1][0] = x[N-1];   xd[1][1] = 0.0;
   xd[2][0] = x[N-1];   xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ N - 1 ] );

   // assembly coefficient matrices

   uint k = 0;
   CA[1][k] = Stif[k][2][2] ;
   CA[2][k] = Stif[k][1][2] ;
   CB[2][k] = Stif[k][0][2] ;

   k = 1;
   CA[0][k] = Stif[k-1][2][0] + Stif[k-1][2][1];
   CA[1][k] = Stif[k-1][1][0] + Stif[k-1][1][1];
   CA[1][k]+= Stif[k  ][3][0] + Stif[k  ][3][3];
   CA[2][k] = Stif[k  ][2][0] + Stif[k  ][2][3] ;

   CB[1][k] = Stif[k-1][0][0] + Stif[k-1][0][1];   // j=0;
   CB[1][k]+= Stif[k  ][0][0] + Stif[k  ][0][3];   // j=0;
   CB[2][k] = Stif[k  ][1][0] + Stif[k  ][1][3];   // j=1;

   for( uint k = 2; k < N - 1; k++ )
   {  // loop for all elem
      // elem k-1: i=1,2
      CA[0][k]  = Stif[k-1][3][1] + Stif[k-1][3][2];  // j=3;
      CA[1][k]  = Stif[k-1][2][1] + Stif[k-1][2][2];  // j=2;
      CB[0][k]  = Stif[k-1][0][1] + Stif[k-1][0][2];  // j=0;
      CB[1][k]  = Stif[k-1][1][1] + Stif[k-1][1][2];  // j=1;

      // elem k: i=0,3
      CA[1][k] += Stif[k  ][3][0] + Stif[k  ][3][3];  // j=3;
      CA[2][k]  = Stif[k  ][2][0] + Stif[k  ][2][3];  // j=2;
      CB[1][k] += Stif[k  ][0][0] + Stif[k  ][0][3];  // j=0;
      CB[2][k]  = Stif[k  ][1][0] + Stif[k  ][1][3];  // j=1;
   }

   k = N - 1;
   // elem[k-1]: quadrilateral
   CA[0][k]  = Stif[k-1][3][1] + Stif[k-1][3][2];  // j=3;
   CA[1][k]  = Stif[k-1][2][1] + Stif[k-1][2][2];  // j=2;
   CB[0][k]  = Stif[k-1][0][1] + Stif[k-1][0][2];  // j=0;
   CB[1][k]  = Stif[k-1][1][1] + Stif[k-1][1][2];  // j=1;

   // elem[k]: triangle
   CA[1][k] += Stif[k][2][0] + Stif[k][2][1] + Stif[k][2][2];
   CB[1][k] += Stif[k][0][0] + Stif[k][0][1] + Stif[k][0][2];
   CB[2][k]  = Stif[k][1][0] + Stif[k][1][1] + Stif[k][1][2];
   
   US_AstfemMath::clear_3d( N, 4, Stif );
}

void US_Astfem_RSA::ComputeCoefMatrixMovingMeshL(
      double D, double sw2, double** CA, double** CB )
{
   // compute local stiffness matrices
   StiffBase stfb0;
   double*** Stif;       // Local stiffness matrix at each element
   double    xd[4][2];   // coord for verices of quad elem

   US_AstfemMath::initialize_3d( N, 4, 4, &Stif );
   // elem[0]: triangle
   xd[0][0] = x[0];
   xd[0][1] = 0.0;
   xd[1][0] = x[1];  xd[1][1] = 0.0;
   xd[2][0] = x[0];  xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ 0 ] );

   // elem[k]: k=1..(N-2), quadrilateral
   for ( uint k = 1; k < N - 1; k++ )
   {  // loop for all elem
      xd[0][0] = x[k  ];   xd[0][1] = 0.0;
      xd[1][0] = x[k+1];   xd[1][1] = 0.0;
      xd[2][0] = x[k  ];   xd[2][1] = af_params.dt;
      xd[3][0] = x[k-1];   xd[3][1] = af_params.dt;
      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // elem[N-1]: triangle
   xd[0][0] = x[N-1];   xd[0][1] = 0.0;
   xd[1][0] = x[N-1];   xd[1][1] = af_params.dt;
   xd[2][0] = x[N-2];   xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ N - 1 ] );

   // assembly coefficient matrices

   uint k = 0;
   CA[1][0] = Stif[0][2][0] + Stif[0][2][1] + Stif[0][2][2];
   CB[0][0] = Stif[0][0][0] + Stif[0][0][1] + Stif[0][0][2] ;
   CB[1][0] = Stif[0][1][0] + Stif[0][1][1] + Stif[0][1][2] ;

   CA[1][0]+= Stif[1][3][0] + Stif[1][3][3] ;
   CA[2][0] = Stif[1][2][0] + Stif[1][2][3] ;
   CB[1][0]+= Stif[1][0][0] + Stif[1][0][3] ;
   CB[2][0] = Stif[1][1][0] + Stif[1][1][3] ;

   for ( uint k = 1; k < N - 2; k++ )
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
   
   US_AstfemMath::clear_3d( N, 4, Stif );
}

// Given total concentration of a group of components involved,
// find the concentration of each component by equlibrium condition
void US_Astfem_RSA::decompose( struct mfem_initial* C0)
{
   uint num_comp = af_params.role.size();
   
   // Note: all components must be defined on the same radial grids
   uint Npts = C0[ 0 ].radius.size();  

   // Special case:  self-association  n A <--> An
   if ( num_comp == 2 )       // Only 2 components and one association rule
   {
      uint   st0 = af_params.association[ 0 ].stoich[ 0 ];
      uint   st1 = af_params.association[ 0 ].stoich[ 1 ];
      double keq = af_params.association[ 0 ].keq;

      for ( uint j = 0; j < Npts; j++ )
      {
          double c1;
          double ct = C0[ 0 ].concentration[ j ] + C0[ 1 ].concentration[ j ] ;
          
          if ( st0 == 2 && st1 == 1 )                // mono <--> dimer
             c1 = ( sqrt( 1.0 + 4.0 * keq * ct ) - 1.0 ) / ( 2.0 * keq );
          
          else if ( st0 == 3 && st1 == 1 )           // mono <--> trimer
             c1 = US_AstfemMath::cube_root( -ct / keq, 1.0 / keq, 0.0 );
          
          else if ( st0 > 3 && st1 == 1 )           // mono <--> n-mer
             c1 = US_AstfemMath::find_C1_mono_Nmer( st0, keq, ct );
          
          else
          {
             qDebug() << "Warning: invalid stoichiometry in decompose()";
             return;
          }

          double c2 = keq * pow( c1, (double) st0 );

          if ( af_params.role[ 0 ].react[ 0 ] == 1 )    // c1=reactant
          {
              C0[ 0 ].concentration[ j ] = c1 ;
              C0[ 1 ].concentration[ j ] = c2 ;
          }
          else
          {
              C0[ 0 ].concentration[ j ] = c2 ;  // c1=product
              C0[ 1 ].concentration[ j ] = c1 ;
          }
      }
      return;
   }

   // General cases
   double** C1;
   double** C2;    // Array for all component at all radius position

   US_AstfemMath::initialize_2d( num_comp, Npts, &C1 );
   US_AstfemMath::initialize_2d( num_comp, Npts, &C2 );

   for( uint i = 0; i < num_comp; i++ )
   {
      for( uint j = 0; j < Npts; j++ )
      {
          C1[ i ][ j ] = C0[ i ].concentration[ j ];
          C2[ i ][ j ] = C1[ i ][ j ];
      }
   }

   // Estimate max time to reach equilibrium and suitable step size:
   // using e^{-k_min * N * dt ) < 1.e-7
   
   uint   time_max = 100 ;          // maximum number of time steps for reacing equlibrium
   double k_min    = 1.e12;
   double TimeStepSize;             // time step size
   
   for ( int i = 0; i < af_params.association.size(); i++ )
   {
      if ( k_min > af_params.association[ i ].k_off ) 
         k_min = af_params.association[ i ].k_off;
   }

   if ( k_min < 1.e-12 ) k_min = 1.e-12;

   TimeStepSize = - log(1.e-7) / ( k_min * (double) time_max );

   // time loop
   for ( uint ti = 0; ti < time_max; ti++ )
   {
      ReactionOneStep_Euler_imp( Npts, C1, TimeStepSize );

      double diff = 0.0;
      double ct   = 0.0;

      for ( uint i = 0; i < num_comp; i++ )
      {
         for ( uint j = 0; j < Npts; j++ )
         {
             diff        += fabs( C2[ i ][ j ] - C1[ i ][ j ] );
             ct          += fabs( C1[ i ][ j ] );
             C2[ i ][ j ] = C1[ i ][ j ];
         }
      }

      if ( diff < 1.e-5 * ct ) break;
   } // end time steps

   for ( uint i = 0; i < num_comp; i++ )
   {
      for ( uint j = 0; j < Npts; j++ )
      {
          C0[ i ].concentration[ j ] = C1[ i ][ j ];
      }
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
      uint Npts, double** C1, double TimeStep )
{
   uint num_comp = af_params.role.size();

   // Special case:  self-association  n A <--> An
   if ( num_comp == 2 )       // only  2 components and one association rule
   {
       double  uhat;

       // Current rule used
       uint   rule = rg[ af_params.rg_index ].association[ 0 ]; 
       uint   st0  = system.assoc_vector[ rule ].stoich[ 0 ];
       uint   st1  = system.assoc_vector[ rule ].stoich[ 1 ];
       double keq  = system.assoc_vector[ rule ].keq;
       double koff = system.assoc_vector[ rule ].k_off;

       for ( uint j = 0; j < Npts; j++ )
       {
          double ct = C1[ 0 ][ j ] + C1[ 1 ][ j ];

          double dva = TimeStep * koff * keq;
          double dvb = TimeStep * koff + 2.;
          double dvc = TimeStep * koff * ct + 2.0 * C1[ 0 ][ j ];
          
          if ( st0 == 2 && st1 == 1 )                // mono <--> dimer
             uhat = 2 * dvc / ( dvb + sqrt( dvb * dvb + 4 * dva * dvc ) );
          
          else if ( st0 ==3 && st1 == 1 )           // mono <--> trimer
             uhat = US_AstfemMath::cube_root( -dvc / dva, dvb / dva, 0.0 );
          
          else if ( st0 > 3 && st1 == 1 )           // mono <--> n-mer
             uhat = US_AstfemMath::find_C1_mono_Nmer( st0, dva / dvb, dvc / dvb);
         
          else
          {
             qDebug() << "Warning: invalid stoichiometry in decompose()";
             return;
          }

          if ( af_params.role[ 0 ].react[ 0 ] == 1 )   // c1=reactant
          {
              C1[ 0 ][ j ] = 2 * uhat - C1[ 0 ][ j ];
              C1[ 1 ][ j ] = ct - C1[ 0 ][ j ];
          }
          else
          {
              C1[ 1 ][ j ] = 2 * uhat - C1[ 1 ][ j ];
              C1[ 0 ][ j ] = ct - C1[ 1 ][ j ];
          }
      }
       return;
   }

   // General cases
   const uint iter_max = 20;      // maximum number of Newton iteration allowed
   
   double **A, *y0, *delta_n, *b, diff;

   y0      = new double [ num_comp ];
   delta_n = new double [ num_comp ];
   b       = new double [ num_comp ];
   
   US_AstfemMath::initialize_2d( num_comp, num_comp, &A );

   for ( uint j = 0; j < Npts; j++ )
   {
      double ct = 0.0;
      
      for ( uint i = 0; i < num_comp; i++ )
      {
         y0[ i ]      = C1[ i ][ j ];
         delta_n[ i ] = 0.0;
         ct          += fabs( y0[ i ] );
      }

      for( uint iter = 0; iter < iter_max; iter++ ) // Newton iteration
      {
         for ( uint i = 0; i < num_comp; i++ ) 
            y0[ i ]=C1[ i ][ j ] + delta_n[ i ];

         Reaction_dydt( y0, b );                  // b=dy/dt

         Reaction_dfdy( y0, A );                  // A=df/dy

         for ( uint i = 0; i < num_comp; i++ )
         {
            for ( uint k = 0; k < num_comp; k++ ) A[ i ][ k ] *= ( -TimeStep );
            
            A[ i ][ i ] += 2.0;
            b[ i ]       = - ( 2 * delta_n[ i ] - TimeStep * b[ i ] );
         }

         if ( US_AstfemMath::GaussElim( num_comp, A, b ) == -1 )
         {
            qDebug() << "Matrix singular in Reaction_Euler_imp: model 12";
            break;
         }
         else
         {
            diff = 0.0;
            
            for ( uint i = 0; i < num_comp; i++ )
            {
               delta_n[ i ] += b[ i ];
               diff         += fabs( delta_n[ i ] );
            }
         }

         if ( diff < 1.e-7 * ct ) break;
      } // End of Newton iteration;

      for ( uint i = 0; i < num_comp; i++ ) C1[ i ][ j ] += delta_n[ i ];

   } // End of j (pts)

   US_AstfemMath::clear_2d( num_comp, A );

   delete [] b;
   delete [] delta_n;
   delete [] y0;
}

void US_Astfem_RSA::Reaction_dydt( double* y, double* yt )
{
    uint    num_comp  = rg[ af_params.rg_index ].GroupComponent.size();
    uint    num_rule  = rg[ af_params.rg_index ].association.size();
    double* Q         = new double [num_rule];
    
    for( uint m = 0; m < num_rule; m++ )
    {
        double k_1        = af_params.association[ m ].k_off;
        double k1         = af_params.association[ m ].keq * k_1;
        
        double Q_reactant = 1.0;
        double Q_product  = 1.0;
        
        for( int n = 0; n < af_params.association[ m ].comp.size(); n++ )
        {
           // local index of the n-th component in assoc[rule]
           uint ind_cn = af_params.association[ m ].comp[ n ] ;
           
           // stoich of n-th comp in the rule
           double stn = af_params.association[ m ].stoich[ n ] ;
           
           // comp[n] here is reactant
           if ( af_params.association[ m ].react[n] == 1 ) 
              Q_reactant *= pow( y[ind_cn], stn );
           
           else   
              Q_product *= pow( y[ ind_cn ], stn );
        }

        Q[ m ] = k1 * Q_reactant - k_1 * Q_product;
    }

    for( uint i = 0; i < num_comp; i++ )
    {
        yt[ i ] = 0.0;

        for ( int m = 0; m < af_params.role[ i ].assoc.size(); m++ )
        {
            yt[ i ] += - af_params.role[ i ].react[ m ] * 
                         af_params.role[ i ].st   [ m ] *
                      Q[ af_params.role[ i ].assoc[ m ] ];
        }
    }

    delete [] Q;
}

void US_Astfem_RSA::Reaction_dfdy( double* y, double** dfdy )
{
    double** QC;

    uint num_comp = rg[ af_params.rg_index ].GroupComponent.size();
    uint num_rule = rg[ af_params.rg_index ].association.size();

    US_AstfemMath::initialize_2d( num_rule, num_comp, &QC );

    for ( uint m=0; m < num_rule; m++ )
    {
        double k_1  = af_params.association[m].k_off;
        double k1   = af_params.association[m].keq * k_1;

        for ( uint j = 0; j < num_comp; j++ )
        {
           double Q_reactant = 1.0;
           double Q_product  = 1.0;
           double deriv_r    = 0.0;
           double deriv_p    = 0.0;
           
           for( int n = 0; n < af_params.association[ m ].comp.size(); n++ )
           {
              // Local index of the n-th component in assoc[rule]
              
              uint ind_cn = af_params.association[ m ].comp[ n ] ;
              
              // Stoich of n-th comp in the rule
              double stn = af_params.association[ m ].stoich[ n ];

              // comp[j] is in the rule
              if ( af_params.association[m].comp[n] == j ) 
              {
                  // comp[n] is reactant
                  if ( af_params.association[ m ].react[ n ] == 1 )   
                     deriv_r = stn * pow( y[ ind_cn ], stn - 1.0 );
                  
                  else      // comp[n] in this rule is reactant
                     deriv_p = stn * pow( y[ ind_cn ], stn - 1.0 );
              }
              else              // comp[j] is not in the rule
              {
                  // comp[n] is reactant
                  if ( af_params.association[ m ].react[ n ] == 1 )
                     Q_reactant *= pow( y[ ind_cn ], stn );
                  
                  else    // comp[n] in this rule is reactant
                     Q_product *= pow( y[ ind_cn ], stn );
              }
           }

           QC[ m ][ j ] = k1 * Q_reactant * deriv_r - k_1 * Q_product * deriv_p;
       }  // C_j
    }    // m-rule

    for ( uint i = 0; i < num_comp; i++ )
    {
        for ( uint j = 0; j < num_comp; j++ )
        {
            dfdy[ i ][ j ] = 0.0;

            for ( int m = 0; m < af_params.role[ i ].assoc.size(); m++ )
            {
                dfdy[ i ][ j ] += - af_params.role[ i ].react[ m ] * 
                                    af_params.role[ i ].st   [ m ] * 
                                QC[ af_params.role[ i ].assoc[ m ] ][ j ];
            }
        }
    }

    US_AstfemMath::clear_2d( num_rule, QC );
}

// *** this is the SNI version of operator scheme

int US_Astfem_RSA::calculate_ra2( double rpm_start, double rpm_stop, 
      mfem_initial* C_init, mfem_data& simdata, bool accel )
{
   uint Mcomp = af_params.s.size();

   simdata.radius.clear();
   simdata.scan.clear();
   mfem_scan simscan;

   // Generate the adaptive mesh
   QList< double > nu;
   nu.clear();

   for ( uint i = 0; i < Mcomp; i++ )
   {
      double sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
      nu << sw2 / af_params.D[ i ];
   }

   mesh_gen( nu, simparams.mesh );

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   uint   NN = af_params.time_steps;
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
         uint   j;
         double sw2 = s_max * sq( rpm_stop * M_PI / 30 );
         xc = m + sw2 * ( NN * dt ) / 3;

         for ( j = 0; j < N - 3; j++ )
         {
            if ( x[ j ] > xc ) break; 
         }
         
         mesh_gen_RefL( j + 1, 4 * j );
      }
      else if ( s_max < 0 )      // all float towards meniscus
      {
         // s_min corresponds to fastest component
         uint   j;
         double sw2 = s_min * sq( rpm_stop * M_PI / 30 ); 
         xc = b + sw2 * ( NN * dt) / 3;
         for ( j = 0; j < N - 3; j++ )
         {
            if ( x[ N - j - 1 ] < xc )  break; 
         }

         mesh_gen_RefL( j + 1, 4 * j );
      }
      else
      {
         qDebug() << "Multicomponent system with sedimentation and "
                     "floating mixed, use uniform mesh";
      }
   }

   for ( uint i = 0; i < N; i++ )
   {
      simdata.radius << x[ i ];
   }

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

      for( uint i = 0; i < Mcomp; i++ )
      {
         double sw2 = 0.0;
         ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA1[ i ], CB1[ i ] );
         
         sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
         ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA2[ i ], CB2[ i ] );
      }
   }
   else     // Constant sedimentation speed
   {
      if ( ! simparams.moving_grid )
      {
         for( uint i = 0; i < Mcomp; i++ )
         {
            double sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
            ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA[ i ], CB[ i ] );
         }
      }
      else  // Moving grid
      {
         if ( s_min > 0)      // All components sedimenting
         {
            for ( uint i = 0; i < Mcomp; i++ )
            {
               double sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );

               // Grid for moving adaptive FEM for faster sedimentation
               
               QList< double > xb;   
               xb.clear();
               xb << m;
               
               for ( uint j = 0; j < N - 1; j++ )
               {
                  double dval = 0.1 * exp( sw2 / af_params.D[ i ] * 
                        ( sq( 0.5 * (x[ j ] + x[ j + 1 ] ) ) - sq( b ) ) / 2 );
                  
                  double alpha = af_params.s[ i ] / s_max * ( 1 - dval ) + dval;
                  xb <<  pow( x[ j ], alpha ) * pow( x[ j + 1 ], ( 1 - alpha) );
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
   double** C0;  // C[m][j]: current/next concentration of m-th component at x_j
   double** C1;  // C[0...Ms-1][0....N-1]:
   
   US_AstfemMath::initialize_2d( Mcomp, N, &C0 );
   US_AstfemMath::initialize_2d( Mcomp, N, &C1 );
   
   // Here we need the interpolate the initial partial 
   // concentration onto new grid x[j]
   
   for( uint i = 0; i < Mcomp; i++ )
   {
      // Interpolate the given C_init vector on the new C0 grid
      US_AstfemMath::interpolate_C0( C_init[ i ], C0[ i ], x ); 
   }
   
   // Total concentration at current and next time step
   double* CT0 = new double [ N ];
   double* CT1 = new double [ N ];
   
   for ( uint j = 0; j < N; j++ )
   {
       CT0[ j ] = 0.0;

       for ( uint i = 0; i < Mcomp; i++ )  CT0[ j ] += C0[ i ][ j ];
       
       CT1[ j ] = CT0[ j ];
   }

   // Time evolution
   double* right_hand_side = new double [N];

   for ( uint kkk = 0; kkk < NN + 2; kkk += 2 )   // two steps in together
   {
      double rpm_current = rpm_start + 
         ( rpm_stop - rpm_start ) * ( kkk + 0.5 ) / NN;

      emit current_speed( (uint) rpm_current);

      simscan.time      = af_params.start_time + kkk * dt;
      simscan.rpm       = (uint) rpm_current;
      w2t_integral     += ( simscan.time - last_time ) * sq(rpm_current * M_PI / 30 );
      last_time         = simscan.time;
      simscan.omega_s_t = w2t_integral;
      
      if ( show_movie )
      {
         emit new_scan( x, CT0 );
         emit new_time( (float) simscan.time );
         qApp->processEvents();
         US_Sleep::msleep( 10 ); // 10 ms to let the display update.
      }

      simscan.conc.clear();
      
      for ( uint j = 0; j < N; j++ ) simscan.conc << CT0[ j ];
      
      simdata.scan << simscan;

      // First half step of sedimentation:
   
      if ( accel ) // need to reconstruct CA and CB by linear interpolation
      {
         double dval = sq( rpm_current / rpm_stop );
         
         for ( uint i = 0; i < Mcomp; i++ )
         {
            for ( uint j1 = 0; j1 < 3; j1++ )
            {
               for ( uint j2 = 0; j2 < N; j2++ )
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

      if ( accel || ! simparams.moving_grid )   // For fixed grid
      {
         for ( uint i = 0; i < Mcomp; i++ )
         {
            right_hand_side[ 0 ] = - CB[ i ][ 1 ][ 0 ] * C0[ i ][ 0 ] 
                                   - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 1 ];
            
            for ( uint j = 1; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ] 
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ] 
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j + 1 ];
            }
            
            uint j = N - 1;
            
            right_hand_side[ j ] = -CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ] 
                                  - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ];
            
            US_AstfemMath::tridiag( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ], 
                                    right_hand_side, C1[ i ], N );
         }
      }
      else // Moving grid
      {
         for ( uint i = 0; i < Mcomp; i++ )
         {
            // Calculate the right hand side vector 
            right_hand_side[ 0 ] = - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 0 ] 
                                   - CB[ i ][ 3 ][ 0 ] * C0[ i ][ 1 ];

            right_hand_side[ 1 ] = - CB[ i ][ 1 ][ 1 ] * C0[ i ][ 0 ] 
                                   - CB[ i ][ 2 ][ 1 ] * C0[ i ][ 1 ] 
                                   - CB[ i ][ 3 ][ 1 ] * C0[ i ][ 2 ];
            
            for ( uint j = 2; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ] [0 ][ j ] * C0[ i ][ j - 2 ]
                                      - CB[ i ] [1 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ] [2 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 3 ][ j ] * C0[ i ][ j + 1 ];
            }

            uint j = N - 1;
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
      
      for ( uint j = 0; j < N; j++ )
      {
         CT1[ j ] = 0.0;

         for ( uint i = 0; i < Mcomp; i++ )
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
         
         for ( uint i = 0; i < Mcomp; i++ )
         {
            for ( uint j1 = 0; j1 < 3; j1++ )
            {
               for ( uint j2 = 0; j2 < N; j2++ )
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

      if ( accel || ! simparams.moving_grid )   // for fixed grid
      {
         for ( uint i = 0; i < Mcomp; i++ )
         {
            right_hand_side[ 0 ] = - CB[ i ][ 1 ][ 0 ] * C0[ i ][ 0 ] 
                                   - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 1 ];
            
            for ( uint j = 1; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]  
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ] 
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j + 1 ];
            }

            uint j = N - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ] 
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::tridiag( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ], 
                                    right_hand_side, C1[ i ], N );
         }
      }
      else // Moving grid
      {
         for ( uint i = 0; i < Mcomp; i++ )
         {
            // Calculate the right hand side vector //
            right_hand_side[ 0 ] = - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 0 ] 
                                   - CB[ i ][ 3 ][ 0 ] * C0[ i ][ 1 ];
            
            right_hand_side[ 1 ] = - CB[ i ][ 1 ][ 1 ] * C0[ i ][ 0 ] 
                                   - CB[ i ][ 2 ][ 1 ] * C0[ i ][ 1 ] 
                                   - CB[ i ][ 3 ][ 1 ] * C0[ i ][ 2 ];

            for ( uint j = 2; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 3 ][ j ] * C0[ i ][ j + 1 ];
            }

            uint j = N - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                   - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::QuadSolver( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ], 
                                       CA[ i ][ 3 ], right_hand_side, C1[ i ], 
                                       N );
         }
      }

      // End of 2nd half step of sendimentation

      // For next 2 time steps
      
      for ( uint j = 0; j < N; j++ )
      {
         CT1[ j ] = 0.0;

         for ( uint i = 0; i < Mcomp; i++ )
         {
            CT1[ j ]    += C1[ i ][ j ];
            C0[ i ][ j ] = C1[ i ][ j ];
         }

         CT0[ j ] = CT1[ j ];
      }
   } // time loop

   emit new_scan( x, CT0 );

   for ( uint i = 0; i < Mcomp; i++ )
   {
     C_init[ i ].radius.clear();
     C_init[ i ].concentration.clear();
     
     for ( uint j = 0; j < N; j++ )
     {
        C_init[ i ].radius        << x[ j ];
        C_init[ i ].concentration << C1[ i ][ j ];
     }
   }

   delete [] CT1;
   delete [] CT0;
   delete [] right_hand_side;

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

void US_Astfem_RSA::GlobalStiff( QList< double >& xb, double** ca, double** cb,
                                 double D, double sw2 )
{
   //  4: global stifness matrix

   // function [CA, CB]=4(x, xb, dt, D, sw2)

   double*** Stif = NULL;
   QList< double > vx;
   
   US_AstfemMath::initialize_3d( N, 6, 2, &Stif );

   // 1st elem
   vx.clear();
   vx << x [ 0 ];
   vx << x [ 1 ];
   vx << x [ 0 ];
   vx << x [ 1 ];
   vx << x [ 2 ];
   vx << xb[ 1 ];
   US_AstfemMath::IntQT1( vx, D, sw2, Stif[ 0 ], af_params.dt );

   // elems in middle
   for ( uint i = 1; i < N - 2; i++ )
   {
      vx.clear();
      vx << x [ i - 1 ];
      vx << x [ i     ];
      vx << x [ i + 1 ];
      vx << x [ i     ];
      vx << x [ i + 1 ];
      vx << x [ i + 2 ];
      vx << xb[ i     ];
      vx << xb[ i + 1 ];
      US_AstfemMath::IntQTm( vx, D, sw2, Stif[ i ], af_params.dt );
   }

   // 2nd last elems
   vx.clear();
   vx << x [ N - 3 ];
   vx << x [ N - 2 ];
   vx << x [ N - 1 ];
   vx << x [ N - 2 ];
   vx << x [ N - 1 ];
   vx << xb[ N - 2 ];
   vx << xb[ N - 1 ];

   US_AstfemMath::IntQTn2( vx, D, sw2, Stif[ N - 2 ], af_params.dt );

   // last elems
   vx.clear();
   vx << x [ N - 2 ];
   vx << x [ N - 1 ];
   vx << x [ N - 1 ];
   vx << xb[ N - 1 ];
   US_AstfemMath::IntQTn1 ( vx, D, sw2, Stif[ N - 1 ], af_params.dt );
   
   // assembly into global stiffness matrix
   ca[0][0] = 0.0;
   ca[1][0] = Stif[0][2][0];
   ca[2][0] = Stif[0][3][0];
   ca[3][0] = Stif[0][4][0];

   cb[0][0] = 0.0;
   cb[1][0] = 0.0;
   cb[2][0] = Stif[0][0][0];
   cb[3][0] = Stif[0][1][0];

   // i=2
   ca[0][1] = Stif[0][2][1];
   ca[1][1] = Stif[0][3][1] + Stif[1][3][0];
   ca[2][1] = Stif[0][4][1] + Stif[1][4][0];
   ca[3][1] =                 Stif[1][5][0];

   cb[0][1] = 0.0;
   cb[1][1] = Stif[0][0][1] + Stif[1][0][0];
   cb[2][1] = Stif[0][1][1] + Stif[1][1][0];
   cb[3][1] =                 Stif[1][2][0];

   // i: middle
   for (  uint i = 2; i < N - 2; i++ )
   {
      ca[0][i] = Stif[i-1][3][1];
      ca[1][i] = Stif[i-1][4][1] + Stif[i][3][0];
      ca[2][i] = Stif[i-1][5][1] + Stif[i][4][0];
      ca[3][i] =                   Stif[i][5][0];

      cb[0][i] = Stif[i-1][0][1];
      cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
      cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
      cb[3][i] =                   Stif[i][2][0];
   }

   // i=n
   uint i = N - 2;
   ca[0][i] = Stif[i-1][3][1];
   ca[1][i] = Stif[i-1][4][1] + Stif[i][3][0];
   ca[2][i] = Stif[i-1][5][1] + Stif[i][4][0];
   ca[3][i] = 0.0;

   cb[0][i] = Stif[i-1][0][1];
   cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
   cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
   cb[3][i] =                   Stif[i][2][0];

   // i=n+1
   i = N - 1;
   ca[0][i] = Stif[i-1][3][1];
   ca[1][i] = Stif[i-1][4][1] + Stif[i][2][0];
   ca[2][i] = 0.0;
   ca[3][i] = 0.0;

   cb[0][i] = Stif[i-1][0][1];
   cb[1][i] = Stif[i-1][1][1] + Stif[i][0][0];
   cb[2][i] = Stif[i-1][2][1] + Stif[i][1][0];
   cb[3][i] = 0.0;

   US_AstfemMath::clear_3d( N, 6, Stif );
}
