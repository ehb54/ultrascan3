//! \file us_lamm_astfvm.cpp

#include "us_lamm_astfvm.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_astfem_rsa.h"
#include "us_settings.h"
#include "us_dataIO.h"

/////////////////////////
//
// Mesh
//
/////////////////////////
US_LammAstfvm::Mesh::Mesh( double xl, double xr, int Nelem, int Opt )
{
   int i;
   dbg_level    = US_Settings::us_debug();

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

      for ( i = 0; i < Ne; i += 2 )  Eid[ i ]    = 1 ;
      for ( i = 1; i < Ne; i += 2 )  Eid[ i ]    = 4 ;
      for ( i = 0; i < Ne; i++ )     RefLev[ i ] = 0;
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

         y[ i ]     = Wt * y2 + Wt2 * ( y1 + y3 );  // smoothed point
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
            i1++;
            i++;
         }
         
         else
         {  // no change
            x1[ i1 + 1 ]   = x[ i + 1 ];
            Eid1[ i1 ]     = Eid[ i ];
            RefLev1[ i1 ]  = RefLev[ i ];
            MeshDen1[ i1 ] = MeshDen[ i ];
            i1++;
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
         if ( Mark[ k ] == 1 ) Ne1++;
     
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
   const double sqrt3    = sqrt( 3.0 );
   const double onethird = 1.0 / 3.0;
   // refinement threshhold: h*|D_3u|^(1/3) > beta
   //double beta  = pow( ErrTol * 6 / ( 2 * sqrt( 3 ) / 72 ), 1. / 3. );
   // Simplify the above:
   double beta  = 6.0 * pow( ErrTol / sqrt3, onethird );

   // coarsening threshhold: h*|D_3u|^(1/3) < alpha
   double alpha = beta / 4;
//DbgLv(3) << "RefMesh: beta alpha" << beta << alpha;

   ComputeMeshDen_D3( u0, u1 );
   Smoothing        ( Ne, MeshDen, 0.7, 4 );
   Unrefine         ( alpha );
   Refine           ( beta  );
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

// create salt data set by solving ideal astfem equations
US_LammAstfvm::SaltData::SaltData( US_Model                amodel,
                                   US_SimulationParameters asparms,
                                   US_DataIO::RawData*     asim_data )
{
   dbg_level  = US_Settings::us_debug();
   model      = amodel;
   simparms   = asparms;
   sa_data    = *asim_data;
DbgLv(2) << "SaltD: sa_data avg.temp." << sa_data.average_temperature();
DbgLv(2) << "SaltD: asim_data avg.temp." << asim_data->average_temperature();

   Nt         = sa_data.scanCount();
   Nx         = sa_data.pointCount();

   model.components.resize( 1 );
   model.components[ 0 ] = amodel.components[ amodel.coSedSolute ];
   model.coSedSolute     = -1;
   // use salt's molar concentration, if possible, for initial concentration
   double conc0          = model.components[ 0 ].molar_concentration;
   conc0                 = ( conc0 < 0.01 ) ?
                           model.components[ 0 ].signal_concentration : conc0;
   conc0                 = ( conc0 < 0.01 ) ? 2.5 : conc0;
//conc0=3100.0;
   model.components[ 0 ].signal_concentration = conc0;

   simparms.meshType     = US_SimulationParameters::ASTFEM;
   simparms.gridType     = US_SimulationParameters::MOVING;

   simparms.radial_resolution =
      ( sa_data.radius( Nx - 1 ) - sa_data.radius( 0 ) ) / (double)( Nx - 1 );
   simparms.firstScanIsConcentration = false;
DbgLv(2) << "SaltD: Nx" << Nx << "r0 rn ri" << sa_data.radius( 0 )
 << sa_data.radius( Nx - 1 ) << simparms.radial_resolution;

   US_Astfem_RSA* astfem = new US_Astfem_RSA( model, simparms );

   //astfem->setTimeInterpolation( true );
   astfem->setTimeInterpolation( false );
   astfem->set_debug_flag( dbg_level );

   for ( int ii = 0; ii < Nt; ii++ )
      for ( int jj = 0; jj < Nx; jj++ )
         sa_data.setValue( ii, jj, 0.0 );

DbgLv(2) << "SaltD: model comps" << model.components.size();
DbgLv(2) << "SaltD: amodel comps" << amodel.components.size();
DbgLv(2) << "SaltD: comp0 s d s_conc" << model.components[0].s
 << model.components[0].D << model.components[0].signal_concentration;
DbgLv(2) << "SaltD:fem: m b  s D  rpm" << simparms.meniscus << simparms.bottom
   << model.components[0].s << model.components[0].D
   << simparms.speed_step[0].rotorspeed;
DbgLv(2) << "SaltD: (0)Nt Nx" << Nt << Nx << "temperature" << sa_data.scanData[0].temperature;
if(dbg_level>0) {
 qDebug() << "SaltD: model";
 model.debug();
 qDebug() << "SaltD: simparms:";
 simparms.debug();
}

   astfem->set_simout_flag( true );         // set flag to output raw simulation
DbgLv(2) << "SaltD: (2)Nx" << Nx << "r0 r1 rm rn" << sa_data.radius(0)
 << sa_data.radius(1) << sa_data.radius(Nx-2) << sa_data.radius(Nx-1);

   astfem->calculate( sa_data );            // solve equations to create data

   Nt         = sa_data.scanCount();
   Nx         = sa_data.pointCount();

DbgLv(2) << "SaltD: (3)Nx" << Nx << "r0 r1 rm rn" << sa_data.radius(0)
 << sa_data.radius(1) << sa_data.radius(Nx-2) << sa_data.radius(Nx-1);
DbgLv(2) << "SaltD: Nt Nx" << Nt << Nx << "temp" << sa_data.scanData[0].temperature;
DbgLv(2) << "SaltD: sa sc0 sec omg" << sa_data.scanData[0].seconds
 << sa_data.scanData[0].omega2t;
   // Limit salt amplitudes to reasonable values
   const double maxsalt = 1e100;
   const double minsalt = -9e99;
   const double minsala = 1e-100;
   const double minsaln = -1e-100;
   int nchg   = 0;
   for ( int ii = 0; ii < Nt; ii++ )
   {
      for ( int jj = 0; jj < Nx; jj++ )
      {
         double saltv  = sa_data.value( ii, jj );
         double salta  = qAbs( saltv );
if(ii<2 && (jj+3)>Nx)
DbgLv(2) << "SaltD:  ii jj" << ii << jj << "saltv salta" << saltv << salta;

         if ( saltv != saltv )
         {  // Nan!!!
            nchg++;
            if ( jj > 0 )
            {
               saltv         = sa_data.value( ii, jj - 1 );
            }
            else
            {
               saltv         = saltv > 0 ? maxsalt : minsalt;
            }
            sa_data.setValue( ii, jj, saltv );
DbgLv(2) << "SaltD:  salt *NaN* ii jj adj-saltv" << ii << jj << saltv;
         }

         else if ( salta > maxsalt  ||  saltv < minsalt )
         {  // Amplitude too large
            nchg++;
            salta         = qMin( maxsalt, qMax( minsalt, salta ) );
            saltv         = saltv > 0 ? salta : -salta;
            sa_data.setValue( ii, jj, saltv );
         }

         else if ( salta < minsala )
         {  // Amplitude too small
            nchg++;
            saltv         = saltv > 0 ? minsala : minsaln;
            sa_data.setValue( ii, jj, saltv );
         }
      }
   }
DbgLv(2) << "SaltD:  salt ampl limit changes" << nchg;

   //xs         = new double [ Nx ];
   //Cs0        = new double [ Nx ];
   //Cs1        = new double [ Nx ];
   xsVec .fill( 0.0, Nx );
   Cs0Vec.fill( 0.0, Nx );
   Cs1Vec.fill( 0.0, Nx );
   xs         = xsVec .data();
   Cs0        = Cs0Vec.data();
   Cs1        = Cs1Vec.data();

   if ( dbg_level > 2 )
   { // save a copy of the salt data set so that it may be plotted for QC
      QString safile  = US_Settings::resultDir() + "/salt_data";
      QDir dir;

      dir.mkpath( safile );
      safile       = safile + "/salt_data.RA.1.S.260.auc";
      US_DataIO::writeRawData( safile, sa_data );
   }

   delete astfem;                           // astfem solver no longer needed

   for ( int jj = 0; jj < Nx; jj++ )
   {  // set salt radius array
      xs[ jj ]     = sa_data.radius( jj );
   }
DbgLv(2) << "SaltD:  Nx" << Nx << "xs sme" << xs[0] << xs[1] << xs[2]
 << xs[Nx/2-1] << xs[Nx/2] << xs[Nx-2+1] << xs[Nx-3] << xs[Nx-2] << xs[Nx-1];
};

US_LammAstfvm::SaltData::~SaltData()
{
   //delete [] xs;
   //delete [] Cs0;
   //delete [] Cs1;
   xsVec .clear();
   Cs0Vec.clear();
   Cs1Vec.clear();
};

void US_LammAstfvm::SaltData::initSalt()
{
   t0         = sa_data.scanData[ 0 ].seconds; // times of 1st 2 salt scans
   t1         = sa_data.scanData[ 1 ].seconds;
   scn        = 2;                             // index to next scan to use
   Nt         = sa_data.scanCount() - 2;       // scan count less two used here
        
   for ( int j = 0; j < Nx; j++ )
   {  // get 1st two salt arrays from 1st two salt scans
      Cs0[ j ]   = sa_data.value( 0, j );
      Cs1[ j ]   = sa_data.value( 1, j );
   }
int k=Nx/2;
int n=Nx-1;
DbgLv(2) << "initSalt: t0 t1" << t0 << t1;
DbgLv(2) << "initSalt:  xs Cs0 Cs1 j" << xs[0] << Cs0[0] << Cs1[0] << 0;
DbgLv(2) << "initSalt:  xs Cs0 Cs1 j" << xs[k] << Cs0[k] << Cs1[k] << k;
DbgLv(2) << "initSalt:  xs Cs0 Cs1 j" << xs[n] << Cs0[n] << Cs1[n] << n;
}

void US_LammAstfvm::SaltData::InterpolateCSalt( int N, double *x, double t,
   double *Csalt )
{
   double* tmp;
DbgLv(2) << "SaltD:ntrp:  N t t1 Nt Nx" << N << t << t1 << Nt << Nx
 << "Cs0N Cs1N" << Cs0[Nx-1] << Cs1[Nx-1];

   while ( ( t1 < t ) && ( Nt > 0 ) ) 
   {  // walk down salt scans until we are straddling desired time value
      t0    = t1;
      tmp   = Cs0;
      Cs0   = Cs1;
      Cs1   = tmp;    // swap Cs0 and Cs1

      t1    = sa_data.scanData[ scn ].seconds;

      for ( int j = 0; j < Nx; j++ )
         Cs1[ j ]   = sa_data.value( scn, j );

      Nt --;             // Nt = time level left
      scn++;
DbgLv(3) << "SaltD:ntrp:      0 t 1" << t0 << t << t1 << "  N s" << Nt << scn;
   }
DbgLv(2) << "SaltD:ntrp:   t0 t t1" << t0 << t << t1 << "  Nt scn" << Nt << scn;

   // interpolate between t0 and t1
   double et1 = ( t - t0 ) / ( t1 - t0 );
   et1        = ( et1 > 1.0 ) ? 1.0 : et1;
   et1        = ( et1 < 0.0 ) ? 0.0 : et1;
   double et0 = 1.0 - et1;

   // interpolate between xs[k-1] and xs[k]
   int k  = 1;
   int Lx = Nx - 1;
int knan=0;

   for ( int j = 0; j < N; j++ )      // loop for all x[m]
   {
      double xj  = x[ j ];
      while ( xj > xs[ k ]  &&  k < Lx ) k++; // radial point

      // linear interpolation
      int m      = k - 1;
      double xik = ( xj - xs[ m ] ) / ( xs[ k ] - xs[ m ] );
      xik        = ( xik > 1.0 ) ? 1.0 : xik;
      xik        = ( xik < 0.0 ) ? 0.0 : xik;
      double xim = 1.0 - xik;
      // interpolate linearly in both time and radius
      Csalt[ j ] = et0 * ( xim * Cs0[ m ] + xik * Cs0[ k ] )
                +  et1 * ( xim * Cs1[ m ] + xik * Cs1[ k ] );
//*DEBUG
double csj=Csalt[j];
if(csj!=csj) {
knan++;
if(knan<20)
DbgLv(2) << "SaltD:ntrp: j,k,xj,xsm,xsk" << j << k << xj << xs[m] << xs[k]
 << "xim xik" << xim << xik << "Csaltj" << Csalt[j] << csj;
}
//*DEBUG
   }

DbgLv(2) << "SaltD:ntrp:  scn" << scn << "N Nx" << N << Nx
 << "Csalt0 CsaltM CsaltN" << Csalt[0] << Csalt[N/2] << Csalt[N-1];
DbgLv(2) << "SaltD:ntrp:    Cs00 Cs0M Cs0N" << Cs0[0] << Cs0[Nx/2] << Cs0[Nx-1];
DbgLv(2) << "SaltD:ntrp:    Cs10 Cs1M Cs1N" << Cs1[0] << Cs1[Nx/2] << Cs1[Nx-1];
}


// construct Lamm solver for finite volume method
US_LammAstfvm::US_LammAstfvm( US_Model&                rmodel,
                              US_SimulationParameters& rsimparms,
                              QObject*                 parent /*=0*/ )
   : QObject( parent ), model( rmodel ), simparams( rsimparms )
{
   comp_x   = 0;           // initial model component index

   dbg_level       = US_Settings::us_debug();
   stopFlag        = false;
   movieFlag       = false;
   double  speed   = simparams.speed_step[ 0 ].rotorspeed;
   double* coefs   = simparams.rotorcoeffs;
   double  stretch = coefs[ 0 ] * speed + coefs[ 1 ] * sq( speed );
   param_m         = simparams.meniscus;
   param_b         = simparams.bottom;
   param_w2        = sq( speed * M_PI / 30.0 );
DbgLv(2) << "LFvm: m b w2 strtch" << param_m << param_b << param_w2 << stretch;
   param_m        -= stretch;
   param_b        += stretch;
DbgLv(2) << "LFvm:  m b" << param_m << param_b;

   MeshSpeedFactor = 1;    // default mesh moving option
   MeshRefineOpt   = 1;    // default mesh refinement option

   NonIdealCaseNo  = 0;    // default case: ideal with constant s, D

   sigma           = 0.;   // default: s=s_0
   delta           = 0.;   // default: D=D_0
   density         = DENS_20W;
   compressib      = 0.0;
   d_coeff[ 0 ]    = DENS_20W;
   v_coeff[ 0 ]    = VISC_20W;
   for ( int ii = 1; ii < 6; ii++ )
   {
      d_coeff[ ii ]   = 0.0;
      v_coeff[ ii ]   = 0.0;
   }

   //err_tol         = 1.0e-4;
   err_tol         = 1.0e-5;
}

// destroy
US_LammAstfvm::~US_LammAstfvm()
{
   if ( NonIdealCaseNo == 2 ) delete saltdata;

   return;
}

// primary method to calculate solutions for all species
int US_LammAstfvm::calculate( US_DataIO::RawData& sim_data )
{
   auc_data = &sim_data;

   // use given data to create form for internal data; zero initial concs.
   load_mfem_data( sim_data, af_data, true );

   // set up to report progress to any listener (e.g., us_astfem_sim)
   int nsteps = af_data.scan.size() * model.components.size();

   if ( model.coSedSolute >= 0 )
   {  // for co-sedimenting, reduce total steps by one scan
      nsteps    -= af_data.scan.size();
   }

#ifndef NO_DB
   emit calc_start( nsteps );
   qApp->processEvents();
#endif

   // update concentrations for each model component
   for ( int ii = 0; ii < model.components.size(); ii++ )
   {
      int rc = solve_component( ii );

      if ( rc != 0 )
         return rc;
   }

#ifndef NO_DB
   emit calc_done();
   qApp->processEvents();
#endif

   // populate user's data set from calculated simulation
   store_mfem_data( sim_data, af_data );

   return 0;
}

// get a solution for a component and update concentrations
int US_LammAstfvm::solve_component( int compx )
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

   int ntcc  = (int)( total_t / dt ) + 1;      // nbr. times in calculations
   int jt    = 0; 
   int nts   = af_data.scan.size();            // nbr. output times (scans)
   int kt    = 0; 
   int ncs   = af_data.radius.size();          // nbr. concentrations each scan
   int N0;
   int N1;
   int N0u;
   int N1u;
   int istep = comp_x * nts;

   double  solut_t  = af_data.scan[ nts - 1 ].time;  // true total time
   int ntc   = (int)( solut_t / dt ) + 1;      // nbr. times in calculations

   QVector< double > conc0;
   QVector< double > conc1;
   QVector< double > rads;

#ifndef NO_DB
   emit comp_progress( compx + 1 );
   qApp->processEvents();
#endif

   if ( nonIdealCaseNo() != 0 )            // set non-ideal case number
   {  // if multiple cases, abort
      return 1;
   }

   if ( NonIdealCaseNo == 3 )
   {  // compressibility:  8-fold smaller delta-t and greater time points
      dt        /= 8.0;
      ntc        = (int)( solut_t / dt ) + 1;
   }

DbgLv(2) << "LAsc:  CX=" << comp_x
 << "  ntcc ntc nts ncs nicase" << ntcc << ntc << nts << ncs << NonIdealCaseNo;
DbgLv(2) << "LAsc:    tot_t dt sol_t" << total_t << dt << solut_t;
DbgLv(2) << "LAsc:     m b s w2" << param_m << param_b << param_s << param_w2;

   if ( NonIdealCaseNo == 2 )
   {  // co-sedimenting
      if ( comp_x == model.coSedSolute )
      {  // if this component is the salt, skip solving for it
         if ( comp_x == 0 )
         {
DbgLv(2) << "NonIdeal2: new saltdata  comp_x" << comp_x;
            saltdata  = new SaltData( model, simparams, auc_data );
            vbar_salt = model.components[ 0 ].vbar20;
         }

         //saltdata->initSalt();
         return 0;
      }

      else if ( compx > model.coSedSolute )
      {  // if beyond the salt component, adjust step count
         istep    -= nts;
      }
   }

   conc0.resize( ncs );
   conc1.resize( ncs );
   rads. resize( ncs );

   Mesh *msh = new Mesh( param_m, param_b, 100, 0 );

   msh->InitMesh( param_s, param_D, param_w2 );
   int mropt = 1;                   // mesh refine option;

   // make settings based on non-ideal case type
   if ( NonIdealCaseNo == 1 )                   // concentration-dependent
   {
      SetNonIdealCase_1( model.components[ comp_x ].sigma,
                         model.components[ comp_x ].delta );
DbgLv(2) << "LAsc:   sigma delta" << model.components[comp_x].sigma
 << model.components[comp_x].delta << "  comp_x" << comp_x;
   }

   else if ( NonIdealCaseNo == 2 )              // co-sedimenting
   {
      SetNonIdealCase_2( );
   }

   else if ( NonIdealCaseNo == 3 )              // compressibility
   {
      SetNonIdealCase_3( mropt, err_tol );
   }

   else
   {
      NonIdealCaseNo = 0;
   }

   SetMeshRefineOpt( mropt );    // mesh refine option
   SetMeshSpeedFactor( 1.0 );    // mesh speed factor


   // get initial concentration for this component
   double sig_conc = model.components[ comp_x ].signal_concentration;

QElapsedTimer timer;
int ktime1=0;
int ktime2=0;
int ktime3=0;
int ktime4=0;
int ktime5=0;
int ktime6=0;
timer.start();
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
DbgLv(2) << "LAsc:  u0 0,1,2...,N" << u0[0] << u0[1] << u0[2]
   << u0[N0u-3] << u0[N0u-2] << u0[N0u-1];

   for ( int jj = 0; jj < ncs; jj++ )
   {  // get output radius vector
      rads[ jj ] = af_data.radius[ jj ];
   }
DbgLv(2) << "LAsc:  r0 rn ncs rsiz" << rads[0] << rads[ncs-1]
   << ncs << af_data.radius.size() << rads.size();

   const double u0max = 1e+100;
   const double u0min = -u0max;

   for ( int jj = 0; jj < N0u; jj++ )
   {
      u0[ jj ]    = qMin( u0max, qMax( u0min, u0[ jj ] ) );
      u1[ jj ]    = qMin( u0max, qMax( u0min, u1[ jj ] ) );
   }
DbgLv(2) << "LAsc:   u0 0,1,2...,N" << u0[0] << u0[1] << u0[2]
   << u0[N0u-3] << u0[N0u-2] << u0[N0u-1];

#ifndef NO_DB
   int    ktinc = 5;                        // signal progress every 5th scan
   if ( nts > 10000 )
      ktinc        = qMax( 2, ( ( nts + 5000 ) / 10000 ) );
   if ( nts < 100 )
      ktinc        = 1;
DbgLv(2) << "LAsc:   nts ktinc" << nts << ktinc;
#endif
   double ts;
   double u_ttl;

   
   QDir dir;
   QString tmpDir = US_Settings::tmpDir();
   if ( ! dir.exists( tmpDir ) ) dir.mkpath( tmpDir );

   QFile ftto( tmpDir + "/tt0-ufvm" );
   if ( dbg_level > 0 )
   {
      if ( ! ftto.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
         qDebug() << "*ERROR* Unable to open tt0-ufvm";
   }
   QTextStream tso( &ftto );
//ntc=ntcc;
   if ( dbg_level > 0 )
      tso << ntc << "\n";

   // main loop for time
   for ( jt = 0, kt = 0; jt < ntc; jt++ )
   {
timer.restart();
      t0    = dt * (double)jt;
      t1    = t0 + dt;
      ts    = af_data.scan[ kt ].time;           // time at output scan
      N0u   = N0 + N0 - 1;
      if ( dbg_level > 0  &&  ( ( jt / 10 ) * 10 ) == jt )
      {
         u_ttl = IntQs( x0, u0, 0, -1, N0-2, 1 );
DbgLv(2) << "LAsc:    jt,kt,t0=" << jt << kt << t0 << " Nv=" << N0
   << "u_ttl=" << u_ttl;
DbgLv(2) << "LAsc:  u0 0,1,2...,N" << u0[0] << u0[1] << u0[2]
   << u0[N0u-3] << u0[N0u-2] << u0[N0u-1];
         tso << QString( "%1 %2 %3\n" )
                   .arg( t0   , 12, 'e', 5, QChar( ' ' ) )
                   .arg( N0 )
                   .arg( u_ttl, 12, 'e', 5, QChar( ' ' ) );
         for ( int j=0; j<N0; j++ )
            tso << QString( "%1\n" ).arg( x0[j], 10, 'e', 6, QChar( ' ' ) );
         tso << QString( "\n" );
         for ( int j=0; j<N0u; j++ )
            tso << QString( "%1\n" ).arg( u0[j], 15, 'e', 7, QChar( ' ' ) );
         tso << QString( "\n\n" );
      }
ktime1+=timer.restart();

      u1p0  = new double [ N0u ];
      LammStepSedDiff_P( t0, dt, N0-1, x0, u0, u1p0 );
ktime2+=timer.restart();

      if ( MeshRefineOpt == 1 )
      {

         msh->RefineMesh( u0, u1p0, err_tol );
ktime3+=timer.restart();

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

ktime4+=timer.restart();
         LammStepSedDiff_C( t0, dt, N0-1, x0, u0, N1-1, x1, u1p, u1 );

         delete [] u1p;
      }

      else
      {
ktime4+=timer.restart();
         LammStepSedDiff_C( t0, dt, N0-1, x0, u0, N1-1, x1, u1p0, u1 );
      }

ktime5+=timer.restart();
      // see if current scan is between calculated times; output scan if so

      if ( ts >= t0  &&  ts <= t1 )
      {  // interpolate concentrations quadratically in x; linearly in time
         double f0 = ( t1 - ts ) / ( t1 - t0 );       // fraction of conc0
         double f1 = ( ts - t0 ) / ( t1 - t0 );       // fraction of conc1

DbgLv(2) << "LAsc: call qI  t0 ts t1" << t0 << ts << t1;
         // do quadratic interpolation to fill out concentrations at time t0
         quadInterpolate( x0, u0, N0, rads, conc0 );

         // do quadratic interpolation to fill out concentrations at time t1
         quadInterpolate( x1, u1, N1, rads, conc1 );
DbgLv(2) << "LAsc:  x0[0] x0[H] x0[N]" << x0[0] << x0[N0/2] << x0[N0-1];
DbgLv(2) << "LAsc:  x1[0] x1[H] x1[N]" << x1[0] << x1[N1/2] << x1[N1-1];
DbgLv(2) << "LAsc:   r[0]  r[H]  r[N]" << rads[0] << rads[ncs/2] << rads[ncs-1];
DbgLv(2) << "LAsc:  u0[0] u0[H] u0[N]" << u0[0] << u0[N0u/2] << u0[N1u-1];
DbgLv(2) << "LAsc:  u1[0] u1[H] u1[N]" << u1[0] << u1[N1u/2] << u1[N1u-1];
DbgLv(2) << "LAsc:  c0[0] c0[H] c0[N]"
 << conc0[0] << conc0[ncs/2] << conc0[ncs-1];
DbgLv(2) << "LAsc:  c1[0] c1[H] c1[N]"
 << conc1[0] << conc1[ncs/2] << conc1[ncs-1];
         double utt0 = IntQs( x0, u0, 0, -1, N0-2, 1 );
         double utt1 = IntQs( x1, u1, 0, -1, N1-2, 1 );
DbgLv(2) << "LAsc:   utt0 utt1" << utt0 << utt1;
DbgLv(2) << "LAsc:stopFlag" << stopFlag;

         double cmax = 0.0;
         double rmax = 0.0;

//f0=1.0; f1=0.0;
         for ( int jj = 0; jj < ncs; jj++ )
         {  // update concentration vector with linear interpolation for time
            af_data.scan[ kt ].conc[ jj ] += ( conc0[ jj ] * f0 +
                                               conc1[ jj ] * f1 );
            double Cm = af_data.scan[ kt ].conc[ jj ];
            if ( Cm > cmax )
            {
               cmax = Cm;
               rmax = af_data.radius[ jj ];
            }
         }
DbgLv(2) << "LAsc: t=" << ts << "Cmax=" << cmax << " r=" << rmax;
DbgLv(2) << "LAsc:   co[0] co[H] co[N]  kt" << af_data.scan[kt].conc[0]
 << af_data.scan[kt].conc[ncs/2] << af_data.scan[kt].conc[ncs-1] << kt;

         istep++;  // bump progress step

#ifndef NO_DB
         if ( ( ( kt / ktinc ) * ktinc ) == kt  ||  ( kt + 1 ) == nts )
         {  // signal progress at every "ktinc'th" scan or final one
            emit calc_progress( istep );
DbgLv(2) << "LAsc: istep" << istep;
            qApp->processEvents();
         }
#endif
#ifndef NO_DB
//      if ( movieFlag  &&
//         ( ( ( kt / ktinc ) * ktinc ) == kt  ||  ( kt + 1 ) == nts ) )
      if ( movieFlag )
      {
         emit new_scan( &af_data.radius, af_data.scan[ kt ].conc.data() );
         emit new_time( t0 );
         qApp->processEvents();
      }
#endif

         kt++;    // bump output time(scan) index

         qApp->processEvents();
         if ( stopFlag )  break;
      }

      delete [] u1p0;

      if ( kt >= nts )
         break;   // if all scans updated, we are done

      qApp->processEvents();
      if ( stopFlag )  break;
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

   if ( dbg_level > 0 )
      ftto.close();

   if ( dbg_level > 0 )
   { // calculate and print the integral of scan curves
      double cimn = 9e+14;
      double cimx = 0.0;
      double ciav = 0.0;
      double dltr = ( af_data.radius[ 1 ] - af_data.radius[ 0 ] ) * 0.5;

      for ( int ii = 0; ii < af_data.scan.size(); ii++ )
      {
         double csum = 0.0;
         double pval = af_data.scan[ ii ].conc[ 0 ];

         for ( int jj = 1; jj < af_data.scan[ ii ].conc.size(); jj++ )
         {
            double cval = af_data.scan[ ii ].conc[ jj ];
            csum       += ( ( cval + pval ) * dltr );
            pval        = cval;
//if ( ii < 19  &&  ( (jj/100)*100 == jj || (jj+5)>nconc ) )
// DbgLv(3) << "   jj cval dltr csum" << jj << cval << dltr << csum;
         }

         DbgLv(2) << "Scan" << ii + 1 << "  Integral" << csum;
         cimn        = ( cimn < csum ) ? cimn : csum;
         cimx        = ( cimx > csum ) ? cimx : csum;
         ciav       += csum;
      }

      ciav       /= (double)af_data.scan.size();
      double cidf = cimx - cimn;
      double cidp = (double)( qRound( 10000.0 * cidf / ciav ) ) / 100.0;
      DbgLv(2) << "  Integral Min Max Mean" << cimn << cimx << ciav;
      DbgLv(2) << "  ( range of" << cidf << "=" << cidp << " percent of mean )";
   }
    
   delete [] x0;  // clean up
   delete [] u0;
   delete [] x1;
   delete [] u1;
   delete msh;
ktime6+=timer.elapsed();
DbgLv(2) << "compx" << comp_x << "times 1-6"
 << ktime1 << ktime2 << ktime3 << ktime4 << ktime5 << ktime6;
   return 0;
}

void US_LammAstfvm::set_buffer( US_Buffer buffer )
{
   density     = buffer.density;             // for compressibility
   compressib  = buffer.compressibility;

   buffer.compositeCoeffs( d_coeff, v_coeff );
DbgLv(2) << "buff d_coeff" << d_coeff[0] << d_coeff[1] << d_coeff[2]
   << d_coeff[3] << d_coeff[4] << d_coeff[5];
DbgLv(2) << "buff v_coeff" << v_coeff[0] << v_coeff[1] << v_coeff[2]
   << v_coeff[3] << v_coeff[4] << v_coeff[5];
}

void US_LammAstfvm::SetNonIdealCase_1( double sigma_k, double delta_k )
{
   sigma       = sigma_k;                   // for concentration dependency
   delta       = delta_k;
}

void US_LammAstfvm::SetNonIdealCase_2( )
{
   if ( comp_x == 0 )
   {
DbgLv(2) << "NonIdeal2: create saltdata";
      saltdata  = new SaltData( model, simparams, auc_data );
      vbar_salt = model.components[ 0 ].vbar20;
   }

DbgLv(2) << "NonIdeal2: initSalt  comp_x" << comp_x;
   saltdata->initSalt();
}

void US_LammAstfvm::SetNonIdealCase_3( int& mropt, double& err_tol )
{
   mropt           = 0;
   err_tol         = 1.0e-5;
}

void US_LammAstfvm::SetMeshSpeedFactor( double speed )
{
   MeshSpeedFactor = speed;
}

void US_LammAstfvm::SetMeshRefineOpt( int Opt )
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
//  Correction step of solving Lamm equation (sedimentation-diffusion only)
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
   int     Ng        = 2 * M1;     // number of x_star points
   int*    ke        = new int    [ Ng ];
   double* MemDouble = new double [ 12 * Ng + 15 ];
   double* flux_p[ 3 ];

   double  dt2     = dt * 0.5;
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
QElapsedTimer timer;
static int ktim1=0;
static int ktim2=0;
static int ktim3=0;
static int ktim4=0;
static int ktim5=0;
static int ktim6=0;
static int ktim7=0;
static int ktim8=0;
timer.start();

   // calculate Sv, Dv at t+dt on xg=(xl, xr)
   for ( int j = 0; j < Ng; j += 2 )
   {
      int j2       = j / 2;
      xg1[ j     ] = x1[ j2 ] * 0.75 + x1[ j2 + 1 ] * 0.25;    // xl
      xg1[ j + 1 ] = x1[ j2 ] * 0.25 + x1[ j2 + 1 ] * 0.75;    // xr
      ug1[ j     ] = ( 3. * u1p[ j ] + 6. * u1p[ j + 1 ] - u1p[ j + 2 ] ) / 8.;
      ug1[ j + 1 ] = ( 3. * u1p[ j + 2 ] + 6. * u1p[ j + 1 ] - u1p[ j ] ) / 8.;
   }

   AdjustSD( t + dt, Ng, xg1, ug1, Sv, Dv );
ktim1+=timer.restart();
DbgLv(2) << "  xg1 0 1 M Nm N" << xg1[0] << xg1[1] << xg1[Ng/2]
   << xg1[Ng-2] << xg1[Ng-1];
DbgLv(2) << "  Sv 0 1 M Nm N" << Sv[0] << Sv[1] << Sv[Ng/2]
   << Sv[Ng-2] << Sv[Ng-1];

   // determine xg0=(xls, xrs)
   for ( int j = 0; j < Ng; j++ )
   {
      double sw2 = Sv[ j ] * param_w2;
      xg0[ j ]   = xg1[ j ] - dt * MeshSpeedFactor * sw2 * xg1[ j ]
                   * exp( -qAbs( sw2 ) * dt / 2 );

      xg0[ j ]   = qMax( param_m, qMin( param_b, xg0[ j ] ) );

      xt[ j ]    = ( xg1[ j ] - xg0[ j ] ) / dt;
   }
DbgLv(2) << "  xg0 0 1 M Nm N" << xg0[0] << xg0[1] << xg0[Ng/2]
   << xg0[Ng-2] << xg0[Ng-1] << "Ng" << Ng;

   // redistribute xgs so that in between [m,b] and in increasing order
   double bl     = param_m;

   for ( int j = 0; j < Ng; j++ )
   {
      int    cnt  = 1;

      while ( xg0[ j ] < bl  &&  ( j + 1 ) < Ng )
      {
         j++;
         cnt++;
      }

      double br   = qMin( xg0[ j ], param_b );

      for ( int jm = 0; jm < cnt; jm++ )
      {  
        xg0[ j - jm ] = br - (double)jm / (double)cnt * ( br - bl );
        xt[  j - jm ] = ( xg1[ j - jm ] - xg0[ j - jm ] ) / dt;
      }

      bl = br;
   }
DbgLv(2) << "   xg0 0 1 M Nm N" << xg0[0] << xg0[1] << xg0[Ng/2]
   << xg0[Ng-2] << xg0[Ng-1] << "Ng" << Ng;
ktim2+=timer.restart();

   // calculate Flux(phi, t+dt) at all xg1
   fun_dphi( -0.5, phiL );  // basis at xi=-1/2
   fun_dphi(  0.5, phiR );

   for ( int j = 0; j < Ng; j++ )
   {
      int    j2 = j / 2;
      double h  = 0.5 * ( x1[ j2 + 1 ] - x1[ j2 ] );

      for ( int jm = 0; jm < 3; jm++ )    // at xl
      {
         flux_p[ jm ][ j ] = ( xt[ j ] - Sv[ j ] * param_w2 * xg1[ j ]
                           - Dv[ j ] / xg1[ j ] ) * phiL[ jm ] 
                           + Dv[ j ] * phiL[ jm + 3 ] / h;
      }
      j++;

      for ( int jm = 0; jm < 3; jm++ )     // at xr
      {
         flux_p[ jm ][ j ] = ( xt[ j ] - Sv[ j ] * param_w2 * xg1[ j ]
                           - Dv[ j ] / xg1[ j ] ) * phiR[ jm ] 
                           + Dv[ j ] * phiR[ jm + 3 ] / h;
      }
   }


   // calculate Sv, Dv at (xg0, t)

   LocateStar( M0 + 1, x0, Ng, xg0, ke, xi );    // position of xg0 on mesh x0

   for ( int j = 0; j < Ng; j++ )
   {
      fun_phi( xi[ j ], phi );

      int j2   = 2 * ke[ j ];
      ug0[ j ] = u0[ j2 ]     * phi[ 0 ]
               + u0[ j2 + 1 ] * phi[ 1 ]
               + u0[ j2 + 2 ] * phi[ 2 ];
   }

   // calculate s, D at xg0 on time t 
ktim3+=timer.restart();
   AdjustSD( t, Ng, xg0, ug0, Sv, Dv );
ktim4+=timer.restart();

   // calculate Flux(u0,t) at all xg0
   // (i) Compute ux at nodes as average of Du from left and right

   double* ux = new double [ M0 + 1 ];     // D_x(u0) at all x0

   for ( int j = 1; j < M0; j++ )         // internal nodes
   {
      int j2   = 2 * j;
      ux[ j ]  = ( ( u0[ j2 - 2 ] - 4. * u0[ j2 - 1 ] + 3. * u0[ j2 ] )
                 / ( x0[ j ] - x0[ j - 1 ] )
                 - ( 3. * u0[ j2 ] - 4. * u0[ j2 + 1 ] + u0[ j2 + 2 ] )
                 / ( x0[ j + 1 ] - x0[ j ] ) ) / 2.;
   }

   int j2   = 2 * M0;
   ux[ 0 ]  = -( 3. * u0[ 0 ] - 4. * u0[ 1 ] + u0[ 2 ] )
              / ( x0[ 1 ] - x0[ 0 ] ) / 2.;
   ux[ M0 ] = ( u0[ j2 - 2 ] - 4. * u0[ j2 - 1 ] + 3. * u0[ j2 ] )
              / ( x0[ M0 ] - x0[ M0 - 1 ] ) / 2.;

   // (ii) flux(u0,t) at all xg0
   for ( int j = 0; j < Ng; j++ )
   {
      double wt  = ( 1. - xi[ j ] ) / 2.;
      double uxs = ux[ ke[ j ]     ] * wt
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
   for ( int i = 0; i <= Ng; i++ )
      Mtx[ i ] = new double [ 5 ];

ktim5+=timer.restart();
   // Assemble the coefficient matrix
   for ( int i = 1; i < Ng; i += 2 ) 
   {
      int    k       = ( i - 1 ) / 2;
      double h       = 0.5 * ( x1[ k + 1 ] - x1[ k ] );
      Mtx[ i ][ 0 ]  = 0.;
      Mtx[ i ][ 1 ]  = h       / 24.
                       + dt2 * ( flux_p[ 0 ][ i - 1 ] - flux_p[ 0 ][ i ] );
      Mtx[ i ][ 2 ]  = h * 22. / 24.
                       + dt2 * ( flux_p[ 1 ][ i - 1 ] - flux_p[ 1 ][ i ] );
      Mtx[ i ][ 3 ]  = h       / 24.
                       + dt2 * ( flux_p[ 2 ][ i - 1 ] - flux_p[ 2 ][ i ] );
      Mtx[ i ][ 4 ]  = 0.;
   }

   for ( int i = 2; i < Ng; i += 2 ) 
   {
      int    k       = i / 2;

      double h       = 0.5 * ( x1[ k ] - x1[ k - 1 ] );
      Mtx[ i ][ 0 ]  =-h      / 24. + dt2 * flux_p[ 0 ][ i - 1 ];
      Mtx[ i ][ 1 ]  = h * 5. / 24. + dt2 * flux_p[ 1 ][ i - 1 ];
      Mtx[ i ][ 2 ]  = h * 8. / 24. + dt2 * flux_p[ 2 ][ i - 1 ];

      h              = 0.5 * ( x1[ k + 1 ] - x1[ k ] );
      Mtx[ i ][ 2 ] += h * 8. / 24. - dt2 * flux_p[ 0 ][ i ];
      Mtx[ i ][ 3 ]  = h * 5. / 24. - dt2 * flux_p[ 1 ][ i ];
      Mtx[ i ][ 4 ]  =-h      / 24. - dt2 * flux_p[ 2 ][ i ];
   }

   int i         = 0;
   double h      = 0.5 * ( x1[ 1 ] - x1[ 0 ] );
   Mtx[ i ][ 2 ] = h * 8. / 24. - dt2 * flux_p[ 0 ][ 0 ];
   Mtx[ i ][ 3 ] = h * 5. / 24. - dt2 * flux_p[ 1 ][ 0 ];
   Mtx[ i ][ 4 ] =-h      / 24. - dt2 * flux_p[ 2 ][ 0 ];

   i             = Ng;
   h             = 0.5 * ( x1[ M1 ] - x1[ M1 - 1 ] );
   Mtx[ i ][ 0 ] =-h      / 24. + dt2 * flux_p[ 0 ][ i - 1 ];
   Mtx[ i ][ 1 ] = h * 5. / 24. + dt2 * flux_p[ 1 ][ i - 1 ];
   Mtx[ i ][ 2 ] = h * 8. / 24. + dt2 * flux_p[ 2 ][ i - 1 ];
   
ktim6+=timer.restart();

   // assemble the right hand side
   i        = 0;
   rhs[ i ] =  IntQs( x0, u0, 0, -1., ke[ i ], xi[ i ] )
               + dt2 * flux_u[ i ];

   for ( int i = 1; i < Ng; i++ ) 
   {
      rhs[i] = IntQs( x0, u0, ke[ i - 1 ], xi[ i - 1 ], ke[ i ], xi[ i ] )
               + dt2 * ( flux_u[ i ] - flux_u[i - 1 ] );
   }

   i        = Ng;
   rhs[ i ] =  IntQs( x0, u0, ke[ i - 1 ], xi[ i - 1 ], M0 - 1,  1. )
               + dt2 * ( - flux_u[ i - 1 ] );

ktim7+=timer.restart();
   LsSolver53( Ng, Mtx, rhs, u1 );
ktim8+=timer.restart();

   for ( int i = 0; i <= Ng; i++ )
      delete [] Mtx[ i ];

   delete [] Mtx;
   delete [] rhs;


   delete [] ke;
   delete [] MemDouble;
DbgLv(2) << " Diff_C times 1-8" << ktim1 << ktim2 << ktim3 << ktim4
   << ktim5 << ktim6 << ktim7 << ktim8;
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
   const double  Tempt  = 293.15;    // temperature in K
   const double  vbar_w = 0.72;
   const double  rho_w  = 0.998234;  //  density of water
   int     jj;
   QVector< double > CsaltVec( Nv );
   double* Csalt;
   double  Cm     = 0.0;
   double  rho;
   double  visc;
   //double  vbar   = 0.251;
   //double  vbar   = 0.72;      // 0.251; 
   //double  vbar   = model.components[ 0 ].vbar20;
   double  vbar   = model.components[ comp_x ].vbar20;
QElapsedTimer timer;
static int kst1=0;
static int kst2=0;

   switch ( NonIdealCaseNo )
   {
      case 0:      // ideal, s=s_0, D=D_0

         for ( jj = 0; jj < Nv; jj++ ) 
         {
            s_adj[ jj ] = param_s ;
            D_adj[ jj ] = param_D ;
         }
         break;

      case 1:      // concentration dependent
         for ( jj = 0; jj < Nv; jj++ ) 
         {
            s_adj[ jj ] = param_s / ( 1. + sigma * u[ jj ] / x[ jj ] );
            D_adj[ jj ] = param_D / ( 1. + delta * u[ jj ] / x[ jj ] );
         }
         break;

      case 2:      // co-sedimenting
         //** salt-protein
timer.start();

         CsaltVec.resize( Nv );
         Csalt   = CsaltVec.data();
  
DbgLv(2) << "NonIdeal2: ntrp Salt";
         saltdata->InterpolateCSalt( Nv, x, t, Csalt );    // Csalt at (x, t)
kst1+=timer.restart();
         {
double rho0=0.0;
double rhom=0.0;
double rhoe=0.0;
double vis0=0.0;
double vism=0.0;
double vise=0.0;
double cms0=0.0;
double cmsm=0.0;
double cmse=0.0;
            double sA     = param_s * 1.00194 / ( 1.0 - vbar * rho_w );
            double dA     = param_D * Tempt * 1.00194 / 293.15;
            double Cmrt   = 0.0;
            double Cmsq   = 0.0;
            double Cmcu   = 0.0;
            double Cmqu   = 0.0;
           
            for ( jj = 0; jj < Nv; jj++ )
            {
               // The calculations below are a more efficient version of the
               //  following original computations:
               //
               //rho  = 0.998234 + Cm*( 12.68641e-2 + Cm*( 1.27445e-3 + 
               //        Cm*( -11.954e-4 + Cm*258.866e-6 ) ) ) + 6.e-6;
               //visc = 1.00194 - 19.4104e-3*sqrt(Cm) + Cm*( -4.07863e-2 + 
               //        Cm*( 11.5489e-3  + Cm*(-21.774e-4) ) ) - 0.00078;
               //s_adj[j] =(1-vbar*rho)*1.00194/((1-vbar_w*rho_w)*visc)*param_s;
               //D_adj[j] =(Tempt*1.00194)/(293.15*visc) * param_D;
      
               Cm         = Csalt[ jj ];            // Salt concentration
               Cmrt       = sqrt( Cm );             //  and powers of it
               Cmsq       = Cm * Cm;
               Cmcu       = Cm * Cmsq;
               Cmqu       = Cm * Cmcu;

               rho        =          d_coeff[ 0 ]
                            + Cmrt * d_coeff[ 1 ]
                            + Cm   * d_coeff[ 2 ]
                            + Cmsq * d_coeff[ 3 ]
                            + Cmcu * d_coeff[ 4 ]
                            + Cmqu * d_coeff[ 5 ];

               visc       =          v_coeff[ 0 ]
                            + Cmrt * v_coeff[ 1 ]
                            + Cm   * v_coeff[ 2 ]
                            + Cmsq * v_coeff[ 3 ]
                            + Cmcu * v_coeff[ 4 ]
                            + Cmqu * v_coeff[ 5 ];
               visc       = qAbs( visc );

               s_adj[ jj ] = sA * ( 1.0 - vbar * rho ) / visc;

               D_adj[ jj ] = dA / visc;
               //D_adj[ jj ] = qAbs( dA / visc );

if(jj==0){rho0=rho;vis0=visc;cms0=Cm;}
if(jj==Nv/2){rhom=rho;vism=visc;cmsm=Cm;}
if(jj==Nv-1){rhoe=rho;vise=visc;cmse=Cm;}
            }
DbgLv(2) << "AdjSD:   Csalt0 CsaltN Cm" << Csalt[0] << Csalt[Nv-1] << Cm;
DbgLv(2) << "AdjSD:    sadj 0 m n" << s_adj[0] << s_adj[Nv/2] << s_adj[Nv-1];
DbgLv(2) << "AdjSD:    Dadj 0 m n" << D_adj[0] << D_adj[Nv/2] << D_adj[Nv-1];
DbgLv(2) << "AdjSD:     rho 0,m,e" << rho0 << rhom << rhoe;
DbgLv(2) << "AdjSD:     visc 0,m,e" << vis0 << vism << vise;
DbgLv(2) << "AdjSD:     Cm   0,m,e" << cms0 << cmsm << cmse;
DbgLv(2) << "AdjSD:      vbar vbar_w rho_w" << vbar << vbar_w << rho_w;
DbgLv(2) << "AdjSD:       cmrt cm^2 cm^3 cm^4" << Cmrt << Cmsq << Cmcu << Cmqu;
DbgLv(2) << "AdjSD:        d_coeff[0] rho" << d_coeff[0] << rhoe;
DbgLv(2) << "AdjSD:        v_coeff[0] vis" << v_coeff[0] << vise;
         }

kst2+=timer.restart();
DbgLv(3) << "AdjSD:  times 1 2" << kst1 << kst2;
         break;

      case 3:      // compressibility
         {
            double phip  = vbar;     // apparent specific volume
            double alpha = 1.0;
            double factn = 0.5 * density * param_w2 * compressib;
            double msq   = param_m * param_m;
            double sA    = 1.0 - vbar * rho_w;

            for ( int jj = 0; jj < Nv; jj++ )
            {
               rho          = density
                              / ( 1.0 - factn * ( x[ jj ] * x[ jj ] - msq ) );
               double beta  = ( 1.0 - phip * rho ) / sA;
               s_adj[ jj ]  = param_s * alpha * beta;
               D_adj[ jj ]  = param_D * alpha;
            }
DbgLv(3) << "AdjSD: compr dens" << compressib << density;
DbgLv(3) << "AdjSD:    factn msq sa" << factn << msq << sA;
DbgLv(3) << "AdjSD:   sadj 0 m n" << s_adj[0] << s_adj[Nv/2] << s_adj[Nv-1];
DbgLv(3) << "AdjSD:   Dadj 0 m n" << D_adj[0] << D_adj[Nv/2] << D_adj[Nv-1];
         }
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

void US_LammAstfvm::fun_phi( double x, double* y )
{
   double x2 = x * x;
   y[ 0 ] = 0.5 * ( x2 - x );
   y[ 1 ] = 1.0 - x2;
   y[ 2 ] = 0.5 * ( x2 + x );
}

void US_LammAstfvm::fun_dphi( double x, double* y )
{
   // quadratic basis
   double x2 = x * x;
   y[ 0 ] = 0.5 * ( x2 - x );
   y[ 1 ] = 1.0 - x2;
   y[ 2 ] = 0.5 * ( x2 + x );

   // derivatives
   y[ 3 ] = x - 0.5;
   y[ 4 ] = -2. * x;
   y[ 5 ] = x + 0.5;
}

void US_LammAstfvm::fun_Iphi( double x, double* y )
{
   double x2  = x * x;
   double x3  = x * x2 / 6.0;
          x2 *= 0.25;
   y[ 0 ] = x3 - x2;
   y[ 1 ] = x  - x3 * 2.0;
   y[ 2 ] = x3 + x2;
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
   double phia[ 3 ];
   double phib[ 3 ];

   fun_Iphi( xia, phia );
   fun_Iphi( xib, phib );

   intgrl = ( x[ 1 ] - x[ 0 ] ) / 2. * (
              u[ 0 ] * ( phib[ 0 ] - phia[ 0 ] ) + 
              u[ 1 ] * ( phib[ 1 ] - phia[ 1 ] ) +
              u[ 2 ] * ( phib[ 2 ] - phia[ 2 ] ) );

   return( intgrl );
}

//////////////////////////////////////////////////////////////////
//
// integrate a piecewise quadratic function defined on mesh *x
// by nodal values *u from xia in elem ka to xib in elem kb
//
//////////////////////////////////////////////////////////////////
double US_LammAstfvm::IntQs( double *x, double *u,
                             int ka, double xia, int kb, double xib )
{
   double intgrl;
   
   if ( ka == kb )
   {
      intgrl =  IntQ( x + ka, u + 2 * ka, xia, xib );
   }

   else       // integral across different elems of mesh x
   {
      intgrl  = IntQ( x + ka, u + 2 * ka, xia, 1.  );
      intgrl += IntQ( x + kb, u + 2 * kb, -1., xib );

      for ( int k = ka + 1; k <= kb - 1; k++ )
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
   double  intgrl;
   double  phi[ 3 ];

   int*    ke  = new int    [ M1 + 1 ];
   double* xi  = new double [ M1 + 1 ];

   LocateStar( M0 + 1, x0, M1 + 1, x1, ke, xi );

   // u1 = u0 at all nodes 
   for ( int j = 0; j <= M1; j++ ) 
   {
      fun_phi( xi[ j ], phi );

      int idx     = 2 * ke[ j ];
      u1[ 2 * j ] = phi[ 0 ] * u0[ idx     ]
                  + phi[ 1 ] * u0[ idx + 1 ]
                  + phi[ 2 ] * u0[ idx + 2 ];
   }

   for ( int j = 0; j < M1; j++ ) 
   {
      int j2 = 2 * j; 

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

  for ( int j = 0; j < m - 1; j += 2 )
  {
     int j1 = j + 1;
     int j2 = j + 2;

     double multi  = -A[ j1 ][ 1 ] / A[ j ][ 2 ];
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

  for ( int j = m - 1; j > 0; j -= 2 )
  {
     int jm  = j - 1;
     int jp  = j + 1;

     x[ j  ] = ( b[ j  ] - A[ j  ][ 3 ] * x[ jp ] )
                         / A[ j  ][ 2 ];

     x[ jm ] = ( b[ jm ] - A[ jm ][ 3 ] * x[ j  ]
                         - A[ jm ][ 4 ] * x[ jp ] )
                         / A[ jm ][ 2 ];
  }

}

// determine the non-ideal case number: 0/1/2/3
int US_LammAstfvm::nonIdealCaseNo()
{
   int rc = 0;

   if ( comp_x > 0 )
      return rc;

   NonIdealCaseNo  = 0;      // ideal

   US_Model::SimulationComponent* sc = &model.components[ comp_x ];

   if ( sc->sigma != 0.0  ||  sc->delta != 0.0 )
   {  // non-zero sigma or delta given:         concentration-dependent
      NonIdealCaseNo = 1;
   }

   if ( model.coSedSolute >= 0 )
   {  // co-sedimentation solute index not -1:  co-sedimenting
      if ( NonIdealCaseNo != 0 )
         rc          = 2;

      NonIdealCaseNo = 2;
   }

   if ( compressib > 0.0 )
   {  // compressibility factor positive:       compressibility
      if ( NonIdealCaseNo != 0 )
         rc          = 3;

      NonIdealCaseNo = 3;
   }
   return rc;
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
   int    jj   = 3;                   // next u0 index to y2
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
         x1   = x3;                   // start x (previous end)
         x3   = x0[ ii++ ];           // end (next) x
         y1   = y3;                   // y at start x
         y2   = u0[ jj++ ];           // y at mid-point
         y3   = u0[ jj++ ];           // y at end (next) x

      }
      x2   = ( x1 + x3 ) * 0.5;    // mid-point x
//y1 /= x1;
//y2 /= x2;
//y3 /= x3;

      // do the quadratic interpolation of this Y (C*r)
      yv    =
         (( ( xv - x2 ) * ( xv - x3 ) ) / ( ( x1 - x2 ) * ( x1 - x3 ) )) * y1 +
         (( ( xv - x1 ) * ( xv - x3 ) ) / ( ( x2 - x1 ) * ( x2 - x3 ) )) * y2 +
         (( ( xv - x1 ) * ( xv - x2 ) ) / ( ( x3 - x1 ) * ( x3 - x2 ) )) * y3;
//y1 *= x1;
//y2 *= x2;
//y3 *= x3;

      // output interpolated concentration with r factor removed (C = (C*r)/r)
      cout[ kk++ ] = yv / xv;
//cout[ kk++ ] = yv;
   }

}

// load MfemData object used internally from caller's RawData object
void US_LammAstfvm::load_mfem_data( US_DataIO::RawData&      edata, 
                                    US_AstfemMath::MfemData& fdata,
                                    bool zeroout ) 
{
   int  nscan  = edata.scanData.size();  // scan count
//   int  nconc  = edata.x.size();         // concentrations count
   int  nconc  = edata.xvalues.size();   // concentrations count
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
#if 0
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
#endif
#if 1
      if ( zeroout )
         fscan->conc.fill( 0.0, nconc ); // if so specified, set concentrations to zero

      else                        // Otherwise, copy concentrations
         fscan->conc        = edata.scanData[ ii ].rvalues;
   }
   fdata.radius       = edata.xvalues;
int nn=fdata.radius.size() - 1;
int mm=nn/2;
DbgLv(2) << "LdDa:  n r0 rm rn" << nn << fdata.radius[0] << fdata.radius[mm] << fdata.radius[nn];
#endif
}

// store MfemData object used internally into caller's RawData object
void US_LammAstfvm::store_mfem_data( US_DataIO::RawData&      edata, 
                                     US_AstfemMath::MfemData& fdata ) 
{
   int  nscan  = fdata.scan.size();     // scan count
   int  nconc  = fdata.radius.size();   // concentrations count

   edata.description = fdata.id;
   edata.cell        = fdata.cell;
   edata.xvalues     = fdata.radius;
   edata.scanData.resize( nscan );      // mirror number of scans

   for ( int ii = 0; ii < nscan; ii++ )
   {  // copy over each scan
      US_AstfemMath::MfemScan* fscan = &fdata.scan    [ ii ];
      US_DataIO::Scan*         escan = &edata.scanData[ ii ];

      escan->temperature = fscan->temperature;
      escan->rpm         = fscan->rpm;
      escan->seconds     = fscan->time;
      escan->omega2t     = fscan->omega_s_t;
      escan->plateau     = fdata.radius[ nconc - 1 ];
      escan->rvalues     = fscan->conc;
   }
}

void US_LammAstfvm::setStopFlag( bool flag )
{
   stopFlag  = flag;
   qApp->processEvents();
DbgLv(2) << "setStopFlag" << stopFlag;
}

void US_LammAstfvm::setMovieFlag( bool flag )
{
   movieFlag = flag;
   qApp->processEvents();
DbgLv(2) << "setMovieFlag" << movieFlag;
}

