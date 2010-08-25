//! \file us_lamm_astfvm.cpp

#include "us_lamm_astfvm.h"
#include "us_math2.h"
#include "us_constants.h"

/////////////////////////
//
// Mesh
//
/////////////////////////
US_LammAstfvm::Mesh::Mesh(double xl, double xr, int Nelem, int Opt)
{
   int i;

   // constants
   MaxRefLev    = 20;
   MonScale     = 1;
   MonCutoff    = 1000;
   SmoothingWt  = 0.7;
   SmoothingCyl = 4;
 
   Ne      = Nelem;
   Nv      = Ne + 1;
   Eid     = new int    [ Ne ];
   RefLev  = new int    [ Ne ];
   Mark    = new int    [ Ne ];
   MeshDen = new double [ Ne ];
   x       = new double [ Nv ];

   // uniform
   if ( Opt == 0 )
   {
      for ( i = 0; i < Nv; i++ ) 
      {
         x[ i ] = xl + (double)i / (double)Ne * ( xr - xl );
      }

      for ( i = 0; i < Ne; i = i + 2 ) Eid[ i ]    = 1 ;
      for ( i = 1; i < Ne; i = i + 2 ) Eid[ i ]    = 4 ;
      for ( i = 0; i < Ne; i++ )       RefLev[ i ] = 0;
    }
}

/////////////////////////
//
// ~Mesh
//
/////////////////////////
US_LammAstfvm::Mesh::~Mesh()
{
    delete [] x;
    delete [] Eid;
    delete [] RefLev;
    delete [] MeshDen;
    delete [] Mark;
}

/////////////////////////
//
// ComputeMeshDen_D3
//
/////////////////////////
void US_LammAstfvm::Mesh::ComputeMeshDen_D3( double *u0, double *u1 )
{
   int     i;
   int     i2;
   double  h;

   double* D20 = new double [ Ne ];
   double* D21 = new double [ Ne ];
   double* D30 = new double [ Nv ];
   double* D31 = new double [ Nv ];

   // 2nd derivative on elems
   for ( i = 0; i < Ne; i++ )
   {
      h        = ( x[ i + 1 ] - x[ i ] ) / 2;
      i2       = i * 2; 
      D20[ i ] = ( u0[ i2 + 2 ] - 2 * u0[ i2 + 1 ] + u0[ i2 ] ) / ( h * h );
      D21[ i ] = ( u1[ i2 + 2 ] - 2 * u1[ i2 + 1 ] + u1[ i2 ] ) / ( h * h );
   }

   // 3rd derivative at nodes
   for ( i = 1; i < Nv - 1; i++ )
   {
      h        = ( x[ i + 1 ] - x[ i - 1 ] ) / 2;
      D30[ i ] = ( D20[ i ] - D20[ i - 1 ] ) / h;
      D31[ i ] = ( D21[ i ] - D21[ i - 1 ] ) / h;
   }

   // 3rd derivative on elems = average D3u at nodes
   // here use D2 to store 3rd order derivatives
   for ( i = 1; i < Ne - 1; i++ )
   {
      D20[ i ] = ( D30[ i + 1 ] + D30[ i ] ) / 2;
      D21[ i ] = ( D31[ i + 1 ] + D31[ i ] ) / 2;
   }

   D20[ 0      ] = D20[ 1 ];    // Extropolate 3rd derivative to end pts
   D21[ 0      ] = D21[ 1 ]; 
   D20[ Ne - 1 ] = D20[ Ne - 2 ];
   D21[ Ne - 1 ] = D21[ Ne - 2 ];

   // level-off and smoothing
   for ( i = 0; i < Ne; i++ )
   {
      MeshDen[ i ] = pow( 1 + MonScale
         * ( fabs( D20[ i ] ) + fabs( D21[ i ] ) ), 0.33333 );

      if ( MeshDen[ i ] > MonCutoff ) MeshDen[ i ] = MonCutoff;
   }

   Smoothing( Ne, MeshDen, SmoothingWt, SmoothingCyl );

   delete [] D20;
   delete [] D21;
   delete [] D30;
   delete [] D31;
}




////////////////////////////////////////////////////////////////////////////
//             y[i] = (1-Wt)/2 * y1 +
// Smoothing:         Wt       * y2 +   // for y1,y2,y3 unsmoothed points 
//                    (1-Wt)/2 * y3;    //  around y[i]
////////////////////////////////////////////////////////////////////////////
void US_LammAstfvm::Mesh::Smoothing( int n, double *y, double Wt, int Cycle )
{
   int     s;
   int     i;
   double  Wt1;
   double  Wt2;

   Wt1        = 1. - Wt;                           // sum of outside pt. weights
   Wt2        = Wt1 * 0.5;                         // weight, each outside pt.

   for ( s = 0; s < Cycle; s++ )
   {
      double y1;
      double y2  = y[ 0 ];
      double y3  = y[ 1 ];

      y[ 0 ]     = Wt * y3 + Wt1 * y2;              // 1st smoothed point

      for ( i = 1; i < n - 1; i++ ) 
      {  // smooth all points except first and last
         y1         = y2;                           // cycle the 3 points
         y2         = y3;                           //  around the current one
         y3         = y[ i + 1 ];

         y[ i ]     = Wt * y2 + Wt2 * ( y3 + y1 );  // smoothed point
      }
    
      y[ n - 1 ] = Wt * y2 + Wt1 * y3;              // last smoothed point
   }
}

/////////////////////////
//
// Unrefine
//
/////////////////////////
void US_LammAstfvm::Mesh::Unrefine( double alpha )
{
   int     i;
   int     i1;
   int     Nv1;
   int     Ne1;
   double* x1;
   int*    Eid1;          // elem id
   int*    RefLev1;       // refinement level of an elem
   double* MeshDen1;      // desired mesh density

   while( 1 )
   {
      // set unref mark on each elem
      for ( i = 0; i < Ne; i++ )  Mark[ i ] = 0;

      Ne1      = Ne;

      for ( i = 0; i < Ne - 1; i++ )
      {
         if ( RefLev[ i ] == RefLev[ i + 1 ] && 
              ( Eid[ i ] / 2 ) == ( Eid[ i + 1 ] / 2 ) && 
              ( x[ i + 1 ] - x[ i     ] ) * MeshDen[ i     ] < alpha &&
              ( x[ i + 2 ] - x[ i + 1 ] ) * MeshDen[ i + 1 ] < alpha )
         {
             Mark[ i ]     = 1;
             Mark[ i + 1 ] = 1;
             Ne1 --;
             i ++; 
         } 
      }

      if ( Ne1 == Ne ) return;     // no more unrefine

      // reallocate memory for the new mesh
      Nv1      = Ne1 + 1;
      Eid1     = new int    [ Ne1 ];
      RefLev1  = new int    [ Ne1 ];
      MeshDen1 = new double [ Ne1 ];
      x1       = new double [ Nv1 ];
      
      // loop to combine eligible elem pairs
      x1[ 0 ]  = x[ 0 ];
      i1       = 0;
      for ( i = 0; i < Ne; i++ )
      {
         if ( Mark[ i ] == 1  &&  Mark[ i + 1 ] == 1 )
         {  // combine two elems
            x1[ i1 + 1 ]   = x[ i + 2 ];
            Eid1[ i1 ]     = Eid[ i ] / 2;
            RefLev1[ i1 ]  = RefLev[ i ] - 1;
            MeshDen1[ i1 ] = ( MeshDen[ i ] + MeshDen[ i + 1 ] ) / 2;
            i1 ++;
            i ++;
         }
         
         else
         {  // no change
            x1[ i1 + 1 ]   = x[ i + 1 ];
            Eid1[ i1 ]     = Eid[ i ];
            RefLev1[ i1 ]  = RefLev[ i ];
            MeshDen1[ i1 ] = MeshDen[ i ];
            i1 ++;
          }
      }
       
      // delete memory for old mesh  
      delete [] x;
      delete [] Eid;
      delete [] RefLev;
      delete [] MeshDen;
      delete [] Mark;

      Ne      = Ne1;
      Nv      = Nv1;
      x       = x1;
      Eid     = Eid1;
      MeshDen = MeshDen1;
      RefLev  = RefLev1;
      Mark    = new int [ Ne1 ];

   } // while

}


/////////////////////////
//
// Refine
//
/////////////////////////
void US_LammAstfvm::Mesh::Refine( double beta )
{
   int     k;
   int     ke;
   int     Nv1;
   int     Ne1;           // number of elements
   int*    Eid1;          // element id
   int*    RefLev1;       // refinement level of an element
   double* MeshDen1;      // desired mesh density
   double* x1;
  
   while( 1 )
   {

      // set marker for elements that need to be refined
      for ( k = 0; k < Ne; k++ )
      {
         if ( ( x[ k + 1 ] - x[ k ] ) * MeshDen[ k ] > beta  &&
                RefLev[ k ] < MaxRefLev )
            Mark[ k ] = 1;

         else
            Mark[ k ] = 0;
      }

      for ( k = 0; k < Ne - 1; k++ )
      {    // RefLev differs at most 2 for nabos
         int rldiff = RefLev[ k ] - RefLev[ k + 1 ];

         if ( rldiff < (-1) )
            Mark[ k     ] = 1; 
        
         else if ( rldiff > 1 )
            Mark[ k + 1 ] = 1;
      }

      Ne1 = Ne;

      for ( k = 0; k < Ne; k++ )
         if ( Mark[ k ] == 1 ) Ne1 ++;
     
      if ( Ne1 == Ne ) return;     // no more elements need refine

      // allocate memory for new mesh
      Nv1      = Ne1 + 1;
      x1       = new double [ Nv1 ];
      Eid1     = new int    [ Ne1 ];
      RefLev1  = new int    [ Ne1 ];
      MeshDen1 = new double [ Ne1 ];

      ke       = 0;
      x1[ 0 ]  = x[ 0 ];

      for ( k = 0; k < Ne; k++ )
      {
         if ( Mark[ k ] == 0 )
         {     // no refine on elem-k
            x1[ ke + 1 ]   = x[ k + 1 ];
            Eid1[ ke ]     = Eid[ k ];
            RefLev1[ ke ]  = RefLev[ k ];
            MeshDen1[ ke ] = MeshDen[ k ];
            ke++;
         }
        
         else
         {      // refine k-th elem
            x1[ ke + 1 ]       = ( x[ k ] + x[ k + 1 ] ) / 2;
            Eid1[ ke ]         = Eid[ k ] * 2;
            RefLev1[ ke ]      = RefLev[ k ] + 1;
            MeshDen1[ ke ]     = MeshDen[ k ];

            x1[ ke + 2 ]       = x[ k + 1 ];
            Eid1[ ke + 1 ]     = Eid1[ ke ] + 1;
            RefLev1[ ke + 1 ]  = RefLev1[ ke ];
            MeshDen1[ ke + 1 ] = MeshDen[ k ];
            ke += 2;
         } // if 
      } // for

      delete [] x;
      delete [] Eid;
      delete [] RefLev;
      delete [] Mark;
      delete [] MeshDen;

      Ne      = Ne1;
      Nv      = Nv1;
      x       = x1;
      Eid     = Eid1;
      RefLev  = RefLev1;
      MeshDen = MeshDen1;
      Mark    = new int [ Ne1 ];
   } // while
}

/////////////////////////
//
// RefineMesh
//
/////////////////////////
void US_LammAstfvm::Mesh::RefineMesh( double *u0, double *u1, double ErrTol )
{
   // refinement threshhold: h*|D_3u|^(1/3) > beta
   double beta  = pow( ErrTol * 6 / ( 2 * sqrt( 3 ) / 72 ), 1. / 3. );

   // coarsening threshhold: h*|D_3u|^(1/3) < alpha
   double alpha = beta / 4;

   ComputeMeshDen_D3( u0, u1 );
   Smoothing( Ne, MeshDen, 0.7, 4 );
   Unrefine( alpha );
   Refine(   beta  );
}
   
/////////////////////////
//
// InitMesh
//
/////////////////////////
void US_LammAstfvm::Mesh::InitMesh( double s, double D, double w2 )
{
   int     j; 
   double  D0;
   double  nu0;
   double  nu1;
   double  nu;
   double  t;
   double  m2;
   double  b2;
   double  x2;
   double* u0;
   double* u1;

   D0  = 1.e-4;
   nu0 = s * w2 / D0;
   nu1 = s * w2 / D;

   m2  = x[ 0 ]  * x[ 0 ];
   b2  = x[ Ne ] * x[ Ne ];

   // FILE *fout;
   // fout = fopen("ti.tmp", "w");

   for ( t = 0; t < 1; t = t + 0.1 ) 
   {
      u0 = new double [ 2 * Nv - 1 ];
      u1 = new double [ 2 * Nv - 1 ];

      nu = pow( nu1, t ) * pow( nu0, 1 - t );

      for ( j = 0; j < Nv; j++ )
      {
         x2          = x[ j ] * x[ j ];
         u0[ 2 * j ] = exp( nu * ( m2 - x2 ) ) * ( nu * nu * m2 );
         u1[ 2 * j ] = exp( nu * ( x2 - b2 ) ) * ( nu * nu * b2 );
      }

      for ( j = 0; j < Ne; j++ )
      {
         x2              = ( x[ j ] + x[ j + 1 ] ) / 2;
         x2              = x2 * x2;
         u0[ 2 * j + 1 ] = exp( nu * ( m2 - x2 ) ) * ( nu * nu * m2 );
         u1[ 2 * j + 1 ] = exp( nu * ( x2 - b2 ) ) * ( nu * nu * b2 );
      }

      RefineMesh( u0, u1, 1.e-4 );

      delete [] u0;
      delete [] u1;
   }
}


US_LammAstfvm::SaltData::SaltData( char *fname, double Moler )
{
   int j;

   SaltMoler = Moler;

   f_salt = fopen( fname, "r" );
   if ( f_salt == NULL )
   {
      qDebug() << "*** Unable to find salt data file" << fname;
      return;
   }

   fscanf( f_salt, "%d", &Nt );
   fscanf( f_salt, "%d", &Nx );

   xs  = new double [Nx];
   Cs0 = new double [Nx];
   Cs1 = new double [Nx];
        
   for ( j = 0; j < Nx; j++ )
      fscanf( f_salt, "%lf", &xs[ j ] );
        
   fscanf( f_salt, "%lf", &t0 );

   for ( j = 0; j < Nx; j++ )
      fscanf( f_salt, "%lf", &Cs0[ j ] );

   fscanf( f_salt, "%lf", &t1 );

   for ( j = 0; j < Nx; j++ )
      fscanf( f_salt, "%lf", &Cs1[ j ] );

   Nt -= 2;   // time level left
};

US_LammAstfvm::SaltData::~SaltData()
{
   fclose(f_salt);
   delete [] xs;
   delete [] Cs0;
   delete [] Cs1;
};

void US_LammAstfvm::SaltData::InterpolateCSalt( int N, double *x, double t,
   double *Csalt )
{
   int     j;
   int     k;
   double  xi;
   double  et;
   double* tmp;

   while ( ( t > t1 ) && ( Nt > 0 ) ) 
   {
      t0    = t1;
      tmp   = Cs0;
      Cs0   = Cs1;
      Cs1   = tmp;    // swap Cs0 and Cs1

      fscanf( f_salt, "%lf", &t1 );

      for ( j = 0; j < Nx; j++ )
         fscanf( f_salt, "%lf", &Cs1[ j ] );

      Nt --;             // Nt = time level left
   }

   // interpolate between t0 and t1
   et    = ( t - t0 ) / ( t1 - t0 );
   et    = ( et > 1.0 ) ? 1.0 : et;
   et    = ( et < 0.0 ) ? 0.0 : et;

   // interpolated between xs_(k-1) and xs_k
   k     = 1;

   for ( j = 0; j < N; j++ )      // loop for all x[m]
   {
      while ( ( x[ j ] > xs[ k ] )  &&  ( k < Nx - 1 ) ) k++;

      // linear interpolation
      xi         = ( x[ j ] - xs[ k - 1 ] ) / ( xs[ k ] - xs[ k - 1 ] );
      xi         = ( xi > 1.0 ) ? 1.0 : xi;
      xi         = ( xi < 0.0 ) ? 0.0 : xi;
      Csalt[ j ] = ( 1 - et ) * ( ( 1 - xi ) * Cs0[ k - 1 ] + xi * Cs0[ k ] )
                      +  et   * ( ( 1 - xi ) * Cs1[ k - 1 ] + xi * Cs1[ k ] );
    }
};


// construct Lamm solver for finite volume method
US_LammAstfvm::US_LammAstfvm( US_Model&                rmodel,
                              US_SimulationParameters& rsimparms,
                              QObject*                 parent /*=0*/ )
   : QObject( parent ), model( rmodel ), simparams( rsimparms )
{
   comp_x   = 0;           // initial model component index

   param_m  = simparams.meniscus;
   param_b  = simparams.bottom;
   param_w2 = sq( simparams.speed_step[ 0 ].rotorspeed * M_PI / 30.0 );

   MeshSpeedFactor = 1;    // default mesh moving option
   MeshRefineOpt   = 1;    // default mesh refinement option

   NonIdealCaseNo  = 0;    // default case: ideal with constant s, D

   sigma           = 0.;   // default: s=s_0
   delta           = 0.;   // default: D=D_0
   
}

// destroy
US_LammAstfvm::~US_LammAstfvm()
{
   if ( NonIdealCaseNo == 2 ) delete saltdata;

   return;
}

// primary method to calculate solutions for all species
void US_LammAstfvm::calculate( US_DataIO2::RawData& sim_data )
{
   // use given data to create form for internal data; zero initial concs.
   load_mfem_data( sim_data, af_data, true );

    // set up to report progress to any listener (e.g., us_astfem_sim)
    int nsteps = af_data.scan.size() * model.components.size();
    emit calc_start( nsteps );

   // update concentrations for each model component
   for ( int ii = 0; ii < model.components.size(); ii++ )
   {
      solve_component( ii );
   }

   emit calc_done();

   // populate user's data set from calculated simulation
   store_mfem_data( sim_data, af_data );
}

// get a solution for a component and update concentrations
void US_LammAstfvm::solve_component( int compx )
{
   comp_x  = compx;

   param_s = model.components[ compx ].s;
   param_D = model.components[ compx ].D;

   double  t0  = 0.;
   double  t1  = 100.;
   double* x0;
   double* u0;
   double* x1;
   double* u1;
   double* u1p0;
   double* u1p;
   double* dtmp;
   double  total_t = ( param_b - param_m ) * 2.0
                   / ( param_s * param_w2 * param_m );
   double  dt      = log( param_b / param_m )
                   / ( param_w2 * param_s * 100.0 );

   int ntc = (int)( total_t / dt ) + 1;      // nbr. times in calculations
   int jt  = 0; 
   int nts = af_data.scan.size();            // nbr. output times (scans)
   int kt  = 0; 
   int ncs = af_data.scan[ 0 ].conc.size();  // nbr. concentrations each scan
   int N0;
   int N1;
   int N0u;
   int N1u;
   int nicase = nonIdealCaseNo();            // non-ideal case number
qDebug() << "LAsc:  CX=" << comp_x
 << "  ntc nts ncs nicase" << ntc << nts << ncs << nicase;
   int istep = comp_x * nts;

   QVector< double > conc0;
   QVector< double > conc1;
   QVector< double > rads;

   emit comp_progress( compx + 1 );

   conc0.resize( ncs );
   conc1.resize( ncs );
   rads. resize( ncs );

   Mesh *msh = new Mesh( param_m, param_b, 100, 0 );

   msh->InitMesh( param_s, param_D, param_w2 );

   // make settings based on non-ideal case type
   if ( nicase == 1 )                   // concentration-dependent
   {
      SetNonIdealCase_1( model.components[ comp_x ].sigma,
                         model.components[ comp_x ].delta );

qDebug() << "LAsc:   sigma delta" << model.components[comp_x].sigma
 << model.components[comp_x].delta << "  comp_x" << comp_x;
   }

   else if ( nicase == 2 )              // co-sedimenting
      SetNonIdealCase_2( (char*)"salt.data", 3.5 );

   else if ( nicase == 3 )              // compressibility
      SetNonIdealCase_3( model.compressibility );

   else
      NonIdealCaseNo = 0;

   SetMeshRefineOpt(   1   );    // mesh refine option
   SetMeshSpeedFactor( 1.0 );    // mesh speed factor

   // get initial concentration for this component
   double sig_conc = model.components[ comp_x ].signal_concentration;

   // initialization
   N0    = msh->Nv;
   N0u   = N0 + N0 - 1;
   x0    = new double [ N0  ];
   u0    = new double [ N0u ];
   N1    = N0;
   N1u   = N0u;
   x1    = new double [ N1  ];
   u1    = new double [ N1u ];

   for ( int jj = 0; jj < N0; jj++ )
   {  // initialize X and U values
      int kk = jj + jj;

      x0[ jj ]   = msh->x[ jj ];              // r value
      x1[ jj ]   = x0[ jj ];
      u0[ kk ]   = msh->x[ jj ] * sig_conc;   // C*r value
      u1[ kk ]   = u0[ kk ];
   }

   for ( int kk = 1; kk < N0u - 1; kk+=2 )
   {  // fill in mid-point U values
      u0[ kk ]   = ( u0[ kk - 1 ] + u0[ kk + 1 ] ) * 0.5;
      u1[ kk ]   = u0[ kk ];
   }
qDebug() << "LAsc:  u0 0,1,2...,N" << u0[0] << u0[1] << u0[2]
   << u0[N0u-3] << u0[N0u-2] << u0[N0u-1];

   for ( int jj = 0; jj < ncs; jj++ )
   {  // get output radius vector
      rads[ jj ] = af_data.radius[ jj ];
   }

   int ktinc = 5;                           // signal progress every 5th scan

   // loop for time
   for ( jt = 0, kt = 0; jt < ntc; jt++ )
   {
      t0    = dt * (double)jt;
      t1    = t0 + dt;

      u1p0  = new double [ N0u ];

      LammStepSedDiff_P( t0, dt, N0-1, x0, u0, u1p0 );

      if ( MeshRefineOpt== 1 )
      {
         msh->RefineMesh( u0, u1p0, 1.0e-4 );

         N1    = msh->Nv;
         N1u   = N1 + N1 - 1;
         u1p   = new double [ N1u ];

         delete [] x1;
         x1    = new double [ N1 ];

         for ( int jj = 0; jj < N1; jj++ )
            x1[ jj ] = msh->x[ jj ];

         ProjectQ( N0-1, x0, u1p0, N1-1, x1, u1p );

         delete [] u1;
         u1    = new double [ N1u ];

         LammStepSedDiff_C( t0, dt, N0-1, x0, u0, N1-1, x1, u1p, u1 );

         delete [] u1p;
      }

      else
      {
         LammStepSedDiff_C( t0, dt, N0-1, x0, u0, N1-1, x1, u1p0, u1 );
      }

      // see if current scan is between calculated times; output scan if so
      double ts  = af_data.scan[ kt ].time;           // time at output scan

      if ( ts >= t0  &&  ts <= t1 )
      {  // interpolate concentrations quadratically; linearly in time
         double f0 = ( t1 - ts ) / ( t1 - t0 );       // fraction of conc0
         double f1 = ( ts - t0 ) / ( t1 - t0 );       // fraction of conc1

qDebug() << "LAsc: call qI  t0 ts t1" << t0 << ts << t1;
         // do quadratic interpolation to fill out concentrations at time t0
         quadInterpolate( x0, u0, N0, rads, conc0 );

         // do quadratic interpolation to fill out concentrations at time t1
         quadInterpolate( x1, u1, N1, rads, conc1 );
qDebug() << "LAsc:  x0[0] x0[H] x0[N]" << x0[0] << x0[N0/2] << x0[N0-1];
qDebug() << "LAsc:  x1[0] x1[H] x1[N]" << x1[0] << x1[N1/2] << x1[N1-1];
qDebug() << "LAsc:   r[0]  r[H]  r[N]" << rads[0] << rads[ncs/2] << rads[ncs-1];
qDebug() << "LAsc:  u0[0] u0[H] u0[N]" << u0[0] << u0[N0u/2] << u0[N1u-1];
qDebug() << "LAsc:  u1[0] u1[H] u1[N]" << u1[0] << u1[N1u/2] << u1[N1u-1];
qDebug() << "LAsc:  c0[0] c0[H] c0[N]"
 << conc0[0] << conc0[ncs/2] << conc0[ncs-1];
qDebug() << "LAsc:  c1[0] c1[H] c1[N]"
 << conc1[0] << conc1[ncs/2] << conc1[ncs-1];

         for ( int jj = 0; jj < ncs; jj++ )
         {  // update concentration vector with linear interpolation for time
            af_data.scan[ kt ].conc[ jj ] += ( conc0[ jj ] * f0 +
                                               conc1[ jj ] * f1 );
         }
qDebug() << "LAsc:   co[0] co[H] co[N]  kt" << af_data.scan[kt].conc[0]
 << af_data.scan[kt].conc[ncs/2] << af_data.scan[kt].conc[ncs-1] << kt;

         istep++;  // bump progress step

         if ( ( ( kt / ktinc ) * ktinc ) == kt  ||  ( kt + 1 ) == nts )
         {  // signal progress at every "ktinc'th" scan or final one
            emit calc_progress( istep );
            qApp->processEvents();
         }

         kt++;    // bump output time(scan) index
      }

      delete [] u1p0;

      if ( kt >= nts )
         break;   // if all scans updated, we are done

      // switch x,u arrays for next iteration
      N0    = N1;
      N0u   = N1u;
      dtmp  = x0;
      x0    = x1;
      x1    = dtmp;
      dtmp  = u0;
      u0    = u1;
      u1    = dtmp;
   }
    
   delete [] x0;  // clean up
   delete [] u0;
   delete [] x1;
   delete [] u1;
   delete msh;
}

void US_LammAstfvm::SetNonIdealCase_1( double sigma_k, double delta_k )
{
   NonIdealCaseNo = 1;

   sigma     = sigma_k;                     // for concentration dependency
   delta     = delta_k;
}

void US_LammAstfvm::SetNonIdealCase_2( char *fname, double Moler )
{
   NonIdealCaseNo = 2;

   saltdata = new SaltData( fname, Moler );  // for co-sedimenting
}

void US_LammAstfvm::SetNonIdealCase_3( double cmpress )
{
   NonIdealCaseNo  = 3;

   cmprssfac       = cmpress;
}

void US_LammAstfvm::SetMeshSpeedFactor( double speed )
{
   MeshSpeedFactor = speed;
}

void US_LammAstfvm::SetMeshRefineOpt(int Opt)
{
   MeshRefineOpt   = Opt;
}

///////////////////////////////////////////////////////////////
//
// LammStepSedDiff_P: 
//  prediction step of solving Lamm equation (sedimentation-diffusion only)
//
// Given solution (x0, u0) at t, to find solution (x1, u1) at time t+dt
// use u1p = u0
//
// M0 = number of elems in x0
// u0 = piecewise quadratic solution at t on mesh x0
// u1 = piecewise quadratic solution at t+dt on mesh x1
//
///////////////////////////////////////////////////////////////
void US_LammAstfvm::LammStepSedDiff_P( double t, double dt, int M0,
   double *x0, double *u0, double *u1 )
{
   LammStepSedDiff_C( t, dt, M0, x0, u0, M0, x0, u0, u1 );
}


///////////////////////////////////////////////////////////////
//
// LammStepSedDiff_C: 
//  Correctiion step of solving Lamm equation (sedimentation-diffusion only)
//
// Given solution (x0, u0) at t, and estimate (x0, u_est) of 
// solution at t+dt, and mesh x1, to find solution (x1, u1)
// at time t+dt
//
// M0, M1 = number of elems in x0, x1
// u0 = piecewise quadratic solution at t on mesh x0
// u1 = piecewise quadratic solution at t+dt on mesh x1
// u1p = piecewise quadratic estimated solution at t+dt on mesh x1
//
///////////////////////////////////////////////////////////////
void US_LammAstfvm::LammStepSedDiff_C( double t, double dt, int M0,
   double *x0, double *u0, int M1, double *x1, double *u1p, double *u1 )
{
   int     i;
   int     j;
   int     j2;
   int     jm;
   int     k;
   int     cnt;
   int     Ng  = 2 * M1;     // number of x_star points

   double  dt2 = dt * 0.5;
   double  uxs;
   double  sw2;
   double  bl;
   double  br;
   double  h;
   double  wt;

   int*    ke        = new int    [ Ng ];
   double* MemDouble = new double [ 12 * Ng + 15 ];
   double* flux_p[ 3 ];

   double* xt      = MemDouble;
   double* xi      = xt          + Ng; 
   double* xg0     = xi          + Ng;
   double* xg1     = xg0         + Ng; 
   double* ug0     = xg1         + Ng;
   double* ug1     = ug0         + Ng; 
   double* Sv      = ug1         + Ng;
   double* Dv      = Sv          + Ng; 
   double* flux_u  = Dv          + Ng; 
   flux_p[ 0 ]     = flux_u      + Ng;
   flux_p[ 1 ]     = flux_p[ 0 ] + Ng;
   flux_p[ 2 ]     = flux_p[ 1 ] + Ng;
   double* phi     = flux_p[ 2 ] + Ng; 
   double* phiL    = phi         + 3;
   double* phiR    = phiL        + 6; 

   // calculate Sv, Dv at t+dt on xg=(xl, xr)
   for ( j = 0; j < Ng; j += 2 )
   {
      j2           = j / 2;
      xg1[ j     ] = x1[ j2 ] * 0.75 + x1[ j2 + 1 ] * 0.25;    // xl
      xg1[ j + 1 ] = x1[ j2 ] * 0.25 + x1[ j2 + 1 ] * 0.75;    // xr
      ug1[ j     ] = ( 3. * u1p[ j ] + 6. * u1p[ j + 1 ] - u1p[ j + 2 ] ) / 8.;
      ug1[ j + 1 ] = ( 3. * u1p[ j + 2 ] + 6. * u1p[ j + 1 ] - u1p[ j ] ) / 8.;
   }

   AdjustSD( t + dt, Ng, xg1, ug1, Sv, Dv );

   // determine xg0=(xls, xrs)
   for ( j = 0; j < Ng; j++ )
   {
      sw2      = Sv[ j ] * param_w2;
      xg0[ j ] = xg1[ j ] - dt * MeshSpeedFactor * sw2 * xg1[ j ]
                 * exp( -fabs( sw2 ) * dt / 2 );

      if      ( xg0[ j ] < param_m ) xg0[ j ] = param_m;
      else if ( xg0[ j ] > param_b ) xg0[ j ] = param_b;

      xt[ j ]  = ( xg1[ j ] - xg0[ j ] ) / dt;
   }

   // redistribute xgs so that in between [m,b] and in increasing order
   bl     = param_m;

   for ( j = 0; j < Ng; j++ )
   {
      cnt    = 1;

      while ( xg0[ j ] < bl) { j++; cnt++; }

      br     = ( xg0[ j ] < param_b ) ? xg0[ j ] : param_b ;

      for ( jm = 0; jm < cnt; jm++ )
      {  
        xg0[ j - jm ] = br - (double)jm / (double)cnt * ( br - bl );
        xt[  j - jm ] = ( xg1[ j - jm ] - xg0[ j - jm ] ) / dt;
      }

      bl = br;
   }

   // calculate Flux(phi, t+dt) at all xg1
   fun_dphi( -0.5, phiL );  // basis at xi=-1/2
   fun_dphi(  0.5, phiR );

   for ( j = 0; j < Ng; j++ )
   {
      j2 = j / 2;
      h  = 0.5 * ( x1[ j2 + 1 ] - x1[ j2 ] );

      for ( jm = 0; jm < 3; jm++ )    // at xl
      {
         flux_p[ jm ][ j ] = ( xt[ j ] - Sv[ j ] * param_w2 * xg1[ j ]
                           - Dv[ j ] / xg1[ j ] ) * phiL[ jm ] 
                           + Dv[ j ] * phiL[ jm + 3 ] / h;
      }
      j++;

      for ( jm = 0; jm < 3; jm++ )     // at xr
      {
         flux_p[ jm ][ j ] = ( xt[ j ] - Sv[ j ] * param_w2 * xg1[ j ]
                           - Dv[ j ] / xg1[ j ] ) * phiR[ jm ] 
                           + Dv[ j ] * phiR[ jm + 3 ] / h;
      }
   }


   // calculate Sv, Dv at (xg0, t)

   LocateStar( M0 + 1, x0, Ng, xg0, ke, xi );    // position of xg0 on mesh x0

   for ( j = 0; j < Ng; j++ )
   {
      fun_phi( xi[ j ], phi );

      j2       = 2 * ke[ j ];
      ug0[ j ] = u0[ j2 ]     * phi[ 0 ]
               + u0[ j2 + 1 ] * phi[ 1 ]
               + u0[ j2 + 2 ] * phi[ 2 ];
   }

   // calculate s, D at xg0 on time t 
   AdjustSD( t, Ng, xg0, ug0, Sv, Dv );

   // calculate Flux(u0,t) at all xg0
   // (i) Compute ux at nodes as average of Du from left and right

   double* ux = new double [ M0 + 1 ];     // D_x(u0) at all x0

   for ( j = 1; j < M0; j++ )         // internal nodes
   {
      j2       = 2 * j;
      ux[ j ]  = ( ( u0[ j2 - 2 ] - 4. * u0[ j2 - 1 ] + 3. * u0[ j2 ] )
                 / ( x0[ j ] - x0[ j - 1 ] )
                 - ( 3. * u0[ j2 ] - 4. * u0[ j2 + 1 ] + u0[ j2 + 2 ] )
                 / ( x0[ j + 1 ] - x0[ j ] ) ) / 2.;
   }

   j2       = 2 * M0;
   ux[ 0 ]  = -( 3. * u0[ 0 ] - 4. * u0[ 1 ] + u0[ 2 ] )
              / ( x0[ 1 ] - x0[ 0 ] ) / 2.;
   ux[ M0 ] = ( u0[ j2 - 2 ] - 4. * u0[ j2 - 1 ] + 3. * u0[ j2 ] )
              / ( x0[ M0 ] - x0[ M0 - 1 ] ) / 2.;

   // (ii) flux(u0,t) at all xg0
   for ( j = 0; j < Ng; j++ )
   {
      wt    = ( 1. - xi[ j ] ) / 2.;
      uxs   = ux[ ke[ j ]     ] * wt
            + ux[ ke[ j ] + 1 ] * ( 1. - wt );    // Du0 at xg0

      if ( ( xg0[ j ] <= ( param_m + 1.e-14 ) ) ||
           ( xg0[ j ] >= ( param_b - 1.e-14 ) ) ) 
      {
         flux_u[ j ] = 0.;
      }

      else
      {
         flux_u[ j ] = -( Sv[ j ] * param_w2 * xg0[ j ] + Dv[ j ] / xg0[ j ] )
                       * ug0[ j ] + Dv[ j ] * uxs + xt[ j ] * ug0[ j ];
      }
   }

   delete [] ux;

   //
   // assemble the linear system of equations
   //
   double** Mtx = new double* [ Ng + 1 ];
   double*  rhs = new double  [ Ng + 1 ];
   for ( i = 0; i <= Ng; i++ )
      Mtx[ i ] = new double [ 5 ];

   // Assemble the coefficient matrix
   for ( i = 1; i < Ng; i += 2 ) 
   {
      k              = ( i - 1 ) / 2;
      h              = 0.5 * ( x1[ k + 1 ] - x1[ k ] );
      Mtx[ i ][ 0 ]  = 0.;
      Mtx[ i ][ 1 ]  = h       / 24.
                       + dt2 * ( flux_p[ 0 ][ i - 1 ] - flux_p[ 0 ][ i ] );
      Mtx[ i ][ 2 ]  = h * 22. / 24.
                       + dt2 * ( flux_p[ 1 ][ i - 1 ] - flux_p[ 1 ][ i ] );
      Mtx[ i ][ 3 ]  = h       / 24.
                       + dt2 * ( flux_p[ 2 ][ i - 1 ] - flux_p[ 2 ][ i ] );
      Mtx[ i ][ 4 ]  = 0.;
   }
   for ( i = 2; i < Ng; i += 2 ) 
   {
      k = (int)(i/2);

      h              = 0.5 * ( x1[ k ] - x1[ k - 1 ] );
      Mtx[ i ][ 0 ]  =-h      / 24. + dt2 * flux_p[ 0 ][ i - 1 ];
      Mtx[ i ][ 1 ]  = h * 5. / 24. + dt2 * flux_p[ 1 ][ i - 1 ];
      Mtx[ i ][ 2 ]  = h * 8. / 24. + dt2 * flux_p[ 2 ][ i - 1 ];

      h              = 0.5 * ( x1[ k + 1 ] - x1[ k ] );
      Mtx[ i ][ 2 ] += h * 8. / 24. - dt2 * flux_p[ 0 ][ i ];
      Mtx[ i ][ 3 ]  = h * 5. / 24. - dt2 * flux_p[ 1 ][ i ];
      Mtx[ i ][ 4 ]  =-h      / 24. - dt2 * flux_p[ 2 ][ i ];
   }

   i             = 0;
   h             = 0.5 * ( x1[ 1 ] - x1[ 0 ] );
   Mtx[ i ][ 2 ] = h * 8. / 24. - dt2 * flux_p[ 0 ][ 0 ];
   Mtx[ i ][ 3 ] = h * 5. / 24. - dt2 * flux_p[ 1 ][ 0 ];
   Mtx[ i ][ 4 ] =-h      / 24. - dt2 * flux_p[ 2 ][ 0 ];

   i             = Ng;
   h             = 0.5 * ( x1[ M1 ] - x1[ M1 - 1 ] );
   Mtx[ i ][ 0 ] =-h      / 24. + dt2 * flux_p[ 0 ][ i - 1 ];
   Mtx[ i ][ 1 ] = h * 5. / 24. + dt2 * flux_p[ 1 ][ i - 1 ];
   Mtx[ i ][ 2 ] = h * 8. / 24. + dt2 * flux_p[ 2 ][ i - 1 ];
   

   // assemble the right hand side
   i        = 0;
   rhs[ i ] =  IntQs( x0, u0, 0, -1., ke[ i ], xi[ i ] )
               + dt2 * flux_u[ i ];

   for ( i = 1; i < Ng; i++ ) 
   {
      rhs[i] = IntQs( x0, u0, ke[ i - 1 ], xi[ i - 1 ], ke[ i ], xi[ i ] )
               + dt2 * ( flux_u[ i ] - flux_u[i - 1 ] );
   }

   i        = Ng;
   rhs[ i ] =  IntQs( x0, u0, ke[ i - 1 ], xi[ i - 1 ], M0 - 1,  1. )
               + dt2 * ( - flux_u[ i - 1 ] );

   LsSolver53( Ng, Mtx, rhs, u1 );

   for ( i = 0; i <= Ng; i++ )
      delete [] Mtx[ i ];

   delete [] Mtx;
   delete [] rhs;


   delete [] ke;
   delete [] MemDouble;
   return;
}



////////////////////////////////////////////////////////////
//
// find in the mesh x0 the index of elem containing each xs, 
// and the xi-coordinates of xs in the elem 
//
// Note: xs has to be in increasing order
//       N0 = number of points in x0
//       Ns = number of points in xs
//
////////////////////////////////////////////////////////////

void US_LammAstfvm::LocateStar( int N0, double *x0, int Ns, double *xs,
                int *ke, double *xi )
{
   int eix = 1;

   for ( int j = 0; j < Ns; j++ )
   {
      while ( xs[ j ] > x0[ eix ]  &&  eix < ( N0 - 1 ) ) eix ++;

      ke[ j ] = eix - 1;
      xi[ j ] = ( xs[ j ]   - x0[ eix - 1 ] )
              / ( x0[ eix ] - x0[ eix - 1 ] ) * 2. - 1.;
   }
}
    
///////////////////////////////////////////////////////////////
//
//  Find the adjusted s and D values according to time t, location x,
//  and concentration C = u/x 
//  (note: the input is u=r*C)
//
///////////////////////////////////////////////////////////////
void US_LammAstfvm::AdjustSD( double t, int Nv, double *x, double *u, 
                              double *s_adj, double *D_adj )
{
   int     j;
   double* Csalt;
   double  Cm;
   double  rho;
   double  visc;
   double  Tempt  = 293.15;    // temperature in K
   double  vbar   =  0.72 ;    // 0.251; 
   double  vbar_w = vbar;
   double  rho_w  = 0.998234;  //  density of water

   switch ( NonIdealCaseNo )
   {
      case 0:      // ideal, s=s_0, D=D_0

         for ( j = 0; j < Nv; j++ ) 
         {
            s_adj[ j ] = param_s ;
            D_adj[ j ] = param_D ;
         }
         break;

      case 1:      // concentration dependent
         for ( j = 0; j < Nv; j++ ) 
         {
            s_adj[ j ] = param_s / ( 1. + sigma * u[ j ] / x[ j ] );
            D_adj[ j ] = param_D / ( 1. + delta * u[ j ] / x[ j ] );
         }
         break;

      case 2:      // co-sedimenting
         //** salt-protein
         Csalt = new double [ Nv ];
  
         saltdata->InterpolateCSalt( Nv, x, t, Csalt);     // Csalt at (x, t)

         for ( j = 0; j < Nv; j++ )
         {
            // salt concentration
            Cm         = Csalt[ j ] * ( saltdata->SaltMoler );
            // Cm = Csalt[j]*2.0;

            rho        = 0.998234 + Cm * ( 12.68641e-2
                                  + Cm * ( 1.27445e-3
                                  + Cm * ( -11.954e-4
                                  + Cm * 258.866e-6 ) ) ) + 6.e-6;

            visc       = 1.00194 - 19.4104e-3 * sqrt( Cm )
                                  + Cm * ( -4.07863e-2
                                  + Cm * ( 11.5489e-3
                                  + Cm * ( -21.774e-4 ) ) ) - 0.00078;
      
            s_adj[ j ] = ( 1 - vbar * rho ) * 1.00194
                         / ( ( 1 - vbar_w * rho_w ) * visc )     * param_s;

            D_adj[ j ] = ( Tempt * 1.00194 ) / ( 293.15 * visc ) * param_D;
         }

         delete [] Csalt;
         break;

      case 3:      // compressibility
         break;

      default:
         qDebug( "invalid case number for non-ideal sedimentation" );
         break;

   } // switch

   return;
}


///////////////////////////////////////////////////////////////////
//
// function [u1] = ProjectQ(x0, u0, x1)
//      Project piecewise quadratic solution u0 at x0 onto mesh x1
//      Note: u1 is defined so that mass conserved locally in each x1-elem
//
// Cao: 07/10/2009
//
///////////////////////////////////////////////////////////////////

void US_LammAstfvm::fun_phi( double x, double *y )
{
   y[ 0 ] = 0.5 * x * ( x - 1 );
   y[ 1 ] = 1. - x * x;
   y[ 2 ] = 0.5 * x * ( x + 1 );
}

void US_LammAstfvm::fun_dphi( double x, double *y )
{
   // quadratic basis
   y[ 0 ] = 0.5 * x * ( x - 1 );
   y[ 1 ] = 1. - x * x;
   y[ 2 ] = 0.5 * x * ( x + 1 );

   // derivatives
   y[ 3 ] = x - 0.5;
   y[ 4 ] = -2. * x;
   y[ 5 ] = x + 0.5;
}

void US_LammAstfvm::fun_Iphi( double x, double *y )
{
   y[ 0 ] = x * x * ( x / 6. - 0.25 );
   y[ 1 ] = x * ( 1. - x * x / 3.);
   y[ 2 ] = x * x * ( x / 6. + 0.25 );
}

////////////////////////////////////////////
//
// integrate a quadratic function defined on (x[0], x[1])
// by nodal values u[0], u[1], u[2] from xi=xia to xib
// here x=x[0]+(x[1]-x[0])*(xi+1)/2
//
////////////////////////////////////////////
double US_LammAstfvm::IntQ( double *x, double *u, double xia, double xib )
{
   double intgrl;
   double Iphi[ 6 ];

   fun_Iphi( xia, Iphi );
   fun_Iphi( xib, Iphi + 3 );

   intgrl = ( x[ 1 ] - x[ 0 ] ) / 2. * (
              u[ 0 ] * ( Iphi[ 3 ] - Iphi[ 0 ] ) + 
              u[ 1 ] * ( Iphi[ 4 ] - Iphi[ 1 ] ) +
              u[ 2 ] * ( Iphi[ 5 ] - Iphi[ 2 ] ) );

   return( intgrl );
}

//////////////////////////////////////////////////////////////////
//
// integrate a piecwise quadratic function defined on mesh *x
// by nodal values *u from xia in elem ka to xib in elem kb
//
//////////////////////////////////////////////////////////////////
double US_LammAstfvm::IntQs( double *x, double *u,
                             int ka, double xia, int kb, double xib )
{
   double intgrl;
   int    k;
   
   if ( ka == kb )
   {
      intgrl =  IntQ( x + ka, u + 2 * ka, xia, xib );
   }

   else       // integral across different elems of mesh x
   {
      intgrl  = IntQ( x + ka, u + 2 * ka, xia, 1.  );
      intgrl += IntQ( x + kb, u + 2 * kb, -1., xib );

      for ( k = ka + 1; k <= kb - 1; k++ )
      {
         int k2  = k + k;
         intgrl += ( x[ k + 1 ] - x[ k ] )
            * ( u[ k2 ] + u[ k2 + 2 ] + 4. * u[ k2 + 1 ] ) / 6.;
      }
   }

   return ( intgrl );
}

/////////////////////////////////////////////////////////////////
//
// Interpolation-projection of a piecewise quadratic u0 on x0
// onto piecewise quadratic on mesh x1
//
// M0, M1 = number of elems in x0 and x1
//
/////////////////////////////////////////////////////////////////
void US_LammAstfvm::ProjectQ( int M0, double *x0, double *u0,
                              int M1, double *x1, double *u1 )
{
   int     idx;
   int     j;
   int     j2;
   double  intgrl;
   double  phi[ 3 ];

   int*    ke  = new int    [ M1 + 1 ];
   double* xi  = new double [ M1 + 1 ];

   LocateStar( M0 + 1, x0, M1 + 1, x1, ke, xi );

   // u1 = u0 at all nodes 
   for ( j = 0; j <= M1; j++ ) 
   {
      fun_phi( xi[ j ], phi );

      idx         = 2 * ke[ j ];
      u1[ 2 * j ] = phi[ 0 ] * u0[ idx ]
                  + phi[ 1 ] * u0[ idx + 1 ]
                  + phi[ 2 ] * u0[ idx + 2 ];
   }

   for ( j = 0; j < M1; j++ ) 
   {
      j2     = 2 * j; 

      intgrl = IntQs( x0, u0, ke[ j ], xi[ j ], ke[ j + 1 ], xi[ j + 1 ] );

      u1[ j2 + 1 ] =  1.5  * intgrl / ( x1[ j + 1 ] - x1[ j ] )
                    - 0.25 * ( u1[ j2 ] + u1[ j2 + 2 ] );
   }

   delete [] ke;
   delete [] xi;
}


///////////////////////////////////////////////////////
//
// LsSolver53
// Matrix A[m+1][5]: with A[i][2]=a_ii diagonal
//                        A[i][0]=A[i][4]=0, for i = odd
// m must be even
//
///////////////////////////////////////////////////////
void US_LammAstfvm::LsSolver53( int m, double **A, double *b, double *x )
{
  int    j;
  int    j1;
  int    j2;
  double multi;

  for ( j = 0; j < m - 1; j += 2 )
  {
     j1 = j + 1;
     j2 = j + 2;

     multi         = -A[ j1 ][ 1 ] / A[ j ][ 2 ];
     A[ j1 ][ 2 ] += multi * A[ j ][ 3 ];
     A[ j1 ][ 3 ] += multi * A[ j ][ 4 ];
     b[ j1 ]      += multi * b[ j ];

     multi         = -A[ j2 ][ 0 ] / A[ j ][ 2 ]; 
     A[ j2 ][ 1 ] += multi * A[ j ][ 3 ]; 
     A[ j2 ][ 2 ] += multi * A[ j ][ 4 ]; 
     b[ j2 ]      += multi * b[ j ]; 

     multi         = -A[ j2 ][ 1 ] / A[ j1 ][ 2 ]; 
     A[ j2 ][ 2 ] += multi * A[ j1 ][ 3 ]; 
     b[ j2 ]      += multi * b[ j1 ]; 
  }
    
  // Back-substitution 
  x[ m ]   = b[ m ] / A[ m ][ 2 ];

  for ( j = m - 1; j > 0; j -= 2 )
  {
     x[ j     ] = ( b[ j ]     - A[ j ][ 3 ]     * x[ j + 1 ] )
                               / A[ j ][ 2 ];

     x[ j - 1 ] = ( b[ j - 1 ] - A[ j - 1 ][ 3 ] * x[ j ]
                               - A[ j - 1 ][ 4 ] * x[ j + 1 ] )
                               / A[ j - 1 ][ 2 ];
  }

}

// determine the non-ideal case number: 0/1/2/3
int US_LammAstfvm::nonIdealCaseNo()
{
   int caseno = 0;      // ideal
   US_Model::SimulationComponent* sc = &model.components[ comp_x ];

   if ( sc->sigma != 0.0  ||  sc->delta != 0.0 )
   {  // non-zero sigma or delta given:         concentration-dependent
      caseno = 1;
   }

   else if ( model.coSedSolute >= 0 )
   {  // co-sedimentation solute index not -1:  co-sedimenting
      caseno = 2;
   }

   else if ( model.compressibility != COMP_25W )
   {  // compressibility factor not water's:    compressibility
      caseno = 3;
   }

   return caseno;
}

// perform quadratic interpolation to fill out full concentration vector
void US_LammAstfvm::quadInterpolate( double* x0, double* u0, int N0,
      QVector< double >& xout, QVector< double >& cout )
{
   int    nout = xout.size();         // output concentrations count
   int    kk   = 0;                   // initial output index
   double xv   = xout[ 0 ];           // first output X
   double yv;                         // output Y

   int    ii   = 2;                   // next x0 index to x3
   int    jj   = 4;                   // next u0 index to y3
   double x1   = x0[ 0 ];             // initial start X
   double x3   = x0[ 1 ];             // initial end X
   double x2   = ( x1 + x3 ) * 0.5;   // initial mid-point X
   double y1   = u0[ 0 ];             // initial start Y
   double y2   = u0[ 1 ];             // initial mid-point Y
   double y3   = u0[ 2 ];             // initial end Y

   cout.resize( nout );

   while ( kk < nout )
   {  // loop to output interpolated concentrations
      xv   = xout[ kk ];              // X for which we need a Y

      while ( xv > x3  &&  ii < N0 )
      {  // if need be, walk up input until between x values
         jj   = ii + ii;              // index to u values
         x1   = x3;                   // start x (previous end)
         x3   = x0[ ii++ ];           // end (next) x
         y1   = u0[ jj - 2 ];         // y at start x
         y2   = u0[ jj - 1 ];         // y at mid-point
         y3   = u0[ jj     ];         // y at end (next) x

         x2   = ( x1 + x3 ) * 0.5;    // mid-point x
      }

      // do the quadratic interpolation of this Y (C*r)
      yv    =
         (( ( xv - x2 ) * ( xv - x3 ) ) / ( ( x1 - x2 ) * ( x1 - x3 ) )) * y1 +
         (( ( xv - x1 ) * ( xv - x3 ) ) / ( ( x2 - x1 ) * ( x2 - x3 ) )) * y2 +
         (( ( xv - x1 ) * ( xv - x2 ) ) / ( ( x3 - x1 ) * ( x3 - x2 ) )) * y3;

      // output interpolated concentration with r factor removed (C = (C*r)/r)
      cout[ kk++ ] = yv / xv;
   }
}

// load MfemData object used internally from caller's RawData object
void US_LammAstfvm::load_mfem_data( US_DataIO2::RawData&     edata, 
                                    US_AstfemMath::MfemData& fdata,
                                    bool zeroout ) 
{
   int  nscan  = edata.scanData.size();  // scan count
   int  nconc  = edata.x.size();         // concentrations count
   fdata.id    = edata.description;
   fdata.cell  = edata.cell;
   fdata.scan  .resize( nscan );         // mirror number of scans
   fdata.radius.resize( nconc );         // mirror number of radius values

   for ( int ii = 0; ii < nscan; ii++ )
   {  // copy over all scans
      US_AstfemMath::MfemScan* fscan = &fdata.scan[ ii ];

      fscan->temperature = edata.scanData[ ii ].temperature;
      fscan->rpm         = edata.scanData[ ii ].rpm;
      fscan->time        = edata.scanData[ ii ].seconds;
      fscan->omega_s_t   = edata.scanData[ ii ].omega2t;
      fscan->conc.resize( nconc );        // mirror number of concentrations

      if ( zeroout )
      {  // if so specified, set concentrations to zero
         fscan->conc.fill( 0.0 );
      }

      else
      {  // otherwise, copy input
         for ( int jj = 0; jj < nconc; jj++ )
         {  // copy all concentrations for a scan
            fscan->conc[ ii ] = edata.value( ii, jj );
         }
      }
   }

   for ( int jj = 0; jj < nconc; jj++ )
   {  // copy all radius values
      fdata.radius[ jj ] = edata.radius( jj );
   }
}

// store MfemData object used internally into caller's RawData object
void US_LammAstfvm::store_mfem_data( US_DataIO2::RawData&     edata, 
                                     US_AstfemMath::MfemData& fdata ) 
{
   int  nscan  = fdata.scan.size();     // scan count
   int  nconc  = fdata.radius.size();   // concentrations count

   edata.description = fdata.id;
   edata.cell        = fdata.cell;
   edata.scanData.resize( nscan );      // mirror number of scans

   for ( int ii = 0; ii < nscan; ii++ )
   {  // copy over each scan
      US_AstfemMath::MfemScan* fscan = &fdata.scan    [ ii ];
      US_DataIO2::Scan*        escan = &edata.scanData[ ii ];

      escan->temperature = fscan->temperature;
      escan->rpm         = fscan->rpm;
      escan->seconds     = fscan->time;
      escan->omega2t     = fscan->omega_s_t;
      escan->plateau     = fdata.radius[ nconc - 1 ];
      escan->readings.resize( nconc );  // mirror number of concentrations

      for ( int jj = 0; jj < nconc; jj++ )
      {  // copy all readings concentrations for this scan
         escan->readings[ jj ] = US_DataIO2::Reading( fscan->conc[ jj ] );
      }
   }

   edata.x.resize( nconc );

   for ( int jj = 0; jj < nconc; jj++ )
   {  // copy radius values
      edata.x[ jj ] = US_DataIO2::XValue( fdata.radius[ jj ] );
   }
}

