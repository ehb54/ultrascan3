//! \file us_lamm_astfvm.cpp

#include <QtWidgets/QFileDialog>
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
US_LammAstfvm::Mesh::Mesh( double xl, double xr, int Nelem, int Opt ) {
   int i;
   dbg_level = US_Settings::us_debug();

   // constants
   MaxRefLev = 20;
   MonScale = 1;
   MonCutoff = 10000;
   SmoothingWt = 0.7;
   SmoothingCyl = 4;

   Ne = Nelem;
   Nv = Ne + 1;
   Eid = new int[ Ne ];
   RefLev = new int[ Ne ];
   Mark = new int[ Ne ];
   MeshDen = new double[ Ne ];
   x = new double[ Nv ];

   // uniform
   if ( Opt == 0 ) {
      for ( i = 0; i < Nv; i++ ) {
         x[ i ] = xl + (double) i / (double) Ne * ( xr - xl );
      }

      for ( i = 0; i < Ne; i += 2 ) Eid[ i ] = 1;
      for ( i = 1; i < Ne; i += 2 ) Eid[ i ] = 4;
      for ( i = 0; i < Ne; i++ ) RefLev[ i ] = 0;
   }
}

/////////////////////////
//
// ~Mesh
//
/////////////////////////
US_LammAstfvm::Mesh::~Mesh() {
   delete[] x;
   delete[] Eid;
   delete[] RefLev;
   delete[] MeshDen;
   delete[] Mark;
}

/////////////////////////
//
// ComputeMeshDen_D3
//
/////////////////////////
void US_LammAstfvm::Mesh::ComputeMeshDen_D3( const double *u0, const double *u1 ) {
   int i;
   int i2;
   double h;

   auto *D20 = new double[Ne];
   auto *D21 = new double[Ne];
   auto *D30 = new double[Nv];
   auto *D31 = new double[Nv];
   // 2nd derivative on elems
   for ( i = 0; i < Ne; i++ ) {
      h = ( x[ i + 1 ] - x[ i ] ) / 2;
      i2 = i * 2;
      D20[ i ] = ( u0[ i2 + 2 ] - 2 * u0[ i2 + 1 ] + u0[ i2 ] ) / ( h * h );
      D21[ i ] = ( u1[ i2 + 2 ] - 2 * u1[ i2 + 1 ] + u1[ i2 ] ) / ( h * h );
   }

   // 3rd derivative at nodes
   for ( i = 1; i < Nv - 1; i++ ) {
      h = ( x[ i + 1 ] - x[ i - 1 ] ) / 2;
      D30[ i ] = ( D20[ i ] - D20[ i - 1 ] ) / h;
      D31[ i ] = ( D21[ i ] - D21[ i - 1 ] ) / h;
   }

   // 3rd derivative on elems = average D3u at nodes
   // here use D2 to store 3rd order derivatives
   for ( i = 1; i < Ne - 1; i++ ) {
      D20[ i ] = ( D30[ i + 1 ] + D30[ i ] ) / 2;
      D21[ i ] = ( D31[ i + 1 ] + D31[ i ] ) / 2;
   }

   D20[ 0 ] = D20[ 1 ];    // Extrapolate 3rd derivative to end pts
   D21[ 0 ] = D21[ 1 ];
   D20[ Ne - 1 ] = D20[ Ne - 2 ];
   D21[ Ne - 1 ] = D21[ Ne - 2 ];

   // level-off and smoothing
   for ( i = 0; i < Ne; i++ ) {
      MeshDen[ i ] = pow( 1 + MonScale * ( fabs( D20[ i ] ) + fabs( D21[ i ] ) ), 0.33333 );

      if ( MeshDen[ i ] > MonCutoff ) MeshDen[ i ] = MonCutoff;
   }

   Smoothing( Ne, MeshDen, SmoothingWt, SmoothingCyl );

   delete[] D20;
   delete[] D21;
   delete[] D30;
   delete[] D31;
}




////////////////////////////////////////////////////////////////////////////
//             y[i] = (1-Wt)/2 * y1 +
// Smoothing:         Wt       * y2 +   // for y1,y2,y3 unsmoothed points
//                    (1-Wt)/2 * y3;    //  around y[i]
////////////////////////////////////////////////////////////////////////////
void US_LammAstfvm::Mesh::Smoothing( int n, double *y, double Wt, int Cycle ) {
   int s;
   int i;
   double Wt1;
   double Wt2;
   Wt1 = 1. - Wt;                           // sum of outside pt. weights
   Wt2 = Wt1 * 0.5;                         // weight, each outside pt.

   for ( s = 0; s < Cycle; s++ ) {
      double y1;
      double y2 = y[ 0 ];
      double y3 = y[ 1 ];

      y[ 0 ] = Wt * y3 + Wt1 * y2;              // 1st smoothed point

      for ( i = 1; i < n - 1; i++ ) {  // smooth all points except first and last
         y1 = y2;                           // cycle the 3 points
         y2 = y3;                           //  around the current one
         y3 = y[ i + 1 ];

         y[ i ] = Wt * y2 + Wt2 * ( y1 + y3 );  // smoothed point
      }
    
      y[ n - 1 ] = Wt * y2 + Wt1 * y3;              // last smoothed point
   }
}

/////////////////////////
//
// Unrefine
//
/////////////////////////
void US_LammAstfvm::Mesh::Unrefine( double alpha ) {
   int i;
   int i1;
   int Nv1;
   int Ne1;
   double *x1;
   int *Eid1;          // elem id
   int *RefLev1;       // refinement level of an elem
   double *MeshDen1;      // desired mesh density

   while ( true ) {
      // set unref mark on each elem
      for ( i = 0; i < Ne; i++ ) Mark[ i ] = 0;

      Ne1 = Ne;

      for ( i = 0; i < Ne - 1; i++ ) {
         if ( RefLev[ i ] == RefLev[ i + 1 ] && ( Eid[ i ] / 2 ) == ( Eid[ i + 1 ] / 2 ) &&
              ( x[ i + 1 ] - x[ i ] ) * MeshDen[ i ] < alpha && ( x[ i + 2 ] - x[ i + 1 ] ) * MeshDen[ i + 1 ] < alpha ) {
            Mark[ i ] = 1;
            Mark[ i + 1 ] = 1;
            Ne1--;
            i++;
         }
      }

      if ( Ne1 == Ne ) return;     // no more unrefine

      // reallocate memory for the new mesh
      Nv1 = Ne1 + 1;
      Eid1 = new int[ Ne1 ];
      RefLev1 = new int[ Ne1 ];
      MeshDen1 = new double[ Ne1 ];
      x1 = new double[ Nv1 ];

      // loop to combine eligible elem pairs
      x1[ 0 ] = x[ 0 ];
      i1 = 0;

      for ( i = 0; i < Ne; i++ ) {
         if ( Mark[ i ] == 1 && Mark[ i + 1 ] == 1 ) {  // combine two elems
            x1[ i1 + 1 ] = x[ i + 2 ];
            Eid1[ i1 ] = Eid[ i ] / 2;
            RefLev1[ i1 ] = RefLev[ i ] - 1;
            MeshDen1[ i1 ] = ( MeshDen[ i ] + MeshDen[ i + 1 ] ) / 2;
            i1++;
            i++;
         } else {  // no change
            x1[ i1 + 1 ] = x[ i + 1 ];
            Eid1[ i1 ] = Eid[ i ];
            RefLev1[ i1 ] = RefLev[ i ];
            MeshDen1[ i1 ] = MeshDen[ i ];
            i1++;
         }
      }

      // delete memory for old mesh
      delete[] x;
      delete[] Eid;
      delete[] RefLev;
      delete[] MeshDen;
      delete[] Mark;

      Ne = Ne1;
      Nv = Nv1;
      x = x1;
      Eid = Eid1;
      MeshDen = MeshDen1;
      RefLev = RefLev1;
      Mark = new int[ Ne1 ];

   } // while

}


/////////////////////////
//
// Refine
//
/////////////////////////
void US_LammAstfvm::Mesh::Refine( double beta ) {
   int k;
   int ke;
   int Nv1;
   int Ne1;           // number of elements
   int *Eid1;          // element id
   int *RefLev1;       // refinement level of an element
   double *MeshDen1;      // desired mesh density
   double *x1;

   while ( true ) {

      // set marker for elements that need to be refined
      for ( k = 0; k < Ne; k++ ) {
         if ((x[ k + 1 ] - x[ k ]) * MeshDen[ k ] > beta && RefLev[ k ] < MaxRefLev )
            Mark[ k ] = 1;

         else
            Mark[ k ] = 0;
      }

      for ( k = 0; k < Ne - 1; k++ ) {    // RefLev differs at most 2 for nabos
         int rldiff = RefLev[ k ] - RefLev[ k + 1 ];

         if ( rldiff < (-1))
            Mark[ k ] = 1;

         else if ( rldiff > 1 )
            Mark[ k + 1 ] = 1;
      }

      Ne1 = Ne;

      for ( k = 0; k < Ne; k++ )
         if ( Mark[ k ] == 1 ) Ne1++;

      if ( Ne1 == Ne ) return;     // no more elements need refine

      // allocate memory for new mesh
      Nv1 = Ne1 + 1;
      x1 = new double[Nv1];
      Eid1 = new int[Ne1];
      RefLev1 = new int[Ne1];
      MeshDen1 = new double[Ne1];

      ke = 0;
      x1[ 0 ] = x[ 0 ];

      for ( k = 0; k < Ne; k++ ) {
         if ( Mark[ k ] == 0 ) {     // no refine on elem-k
            x1[ ke + 1 ] = x[ k + 1 ];
            Eid1[ ke ] = Eid[ k ];
            RefLev1[ ke ] = RefLev[ k ];
            MeshDen1[ ke ] = MeshDen[ k ];
            ke++;
         } else {      // refine k-th elem
            x1[ ke + 1 ] = (x[ k ] + x[ k + 1 ]) / 2;
            Eid1[ ke ] = Eid[ k ] * 2;
            RefLev1[ ke ] = RefLev[ k ] + 1;
            MeshDen1[ ke ] = MeshDen[ k ];

            x1[ ke + 2 ] = x[ k + 1 ];
            Eid1[ ke + 1 ] = Eid1[ ke ] + 1;
            RefLev1[ ke + 1 ] = RefLev1[ ke ];
            MeshDen1[ ke + 1 ] = MeshDen[ k ];
            ke += 2;
         } // if
      } // for

      delete[] x;
      delete[] Eid;
      delete[] RefLev;
      delete[] Mark;
      delete[] MeshDen;

      Ne = Ne1;
      Nv = Nv1;
      x = x1;
      Eid = Eid1;
      RefLev = RefLev1;
      MeshDen = MeshDen1;
      Mark = new int[Ne1];
   } // while
}

/////////////////////////
//
// RefineMesh
//
/////////////////////////
void US_LammAstfvm::Mesh::RefineMesh(double *u0, double *u1, double ErrTol) {
   const double sqrt3 = sqrt(3.0);
   const double onethird = 1.0 / 3.0;
   // refinement threshold: h*|D_3u|^(1/3) > beta
   //double beta  = pow( ErrTol * 6 / ( 2 * sqrt( 3 ) / 72 ), 1. / 3. );
   // Simplify the above:
   double beta = 6.0 * pow(ErrTol / sqrt3, onethird);

   // coarsening threshold: h*|D_3u|^(1/3) < alpha
   double alpha = beta / 4;
//DbgLv(3) << "RefMesh: beta alpha" << beta << alpha;

   ComputeMeshDen_D3(u0, u1);
   Smoothing(Ne, MeshDen, 0.7, 4);
   Unrefine(alpha);
   Refine(beta);
}

/////////////////////////
//
// InitMesh
//
/////////////////////////
void US_LammAstfvm::Mesh::InitMesh(double s, double D, double w2) {
   int j;
   double D0;
   double nu0;
   double nu1;
   double nu;
   double t = 0.0;
   double m2;
   double b2;
   double x2;
   double *u0;
   double *u1;

   D0 = 1.e-4;
   nu0 = s * w2 / D0;
   nu1 = s * w2 / D;

   m2 = x[ 0 ] * x[ 0 ];
   b2 = x[ Ne ] * x[ Ne ];

   // FILE *fout;
   // fout = fopen("ti.tmp", "w");

   for ( int i = 0; i < 10; i++ ) {
      t += 0.1;
      u0 = new double[2 * Nv - 1];
      u1 = new double[2 * Nv - 1];

      nu = pow(nu1, t) * pow(nu0, 1 - t);

      for ( j = 0; j < Nv; j++ ) {
         x2 = x[ j ] * x[ j ];
         u0[ 2 * j ] = exp(nu * (m2 - x2)) * (nu * nu * m2);
         u1[ 2 * j ] = exp(nu * (x2 - b2)) * (nu * nu * b2);
      }

      for ( j = 0; j < Ne; j++ ) {
         x2 = (x[ j ] + x[ j + 1 ]) / 2;
         x2 = x2 * x2;
         u0[ 2 * j + 1 ] = exp(nu * (m2 - x2)) * (nu * nu * m2);
         u1[ 2 * j + 1 ] = exp(nu * (x2 - b2)) * (nu * nu * b2);
      }

      RefineMesh(u0, u1, 1.e-4);

      delete[] u0;
      delete[] u1;
   }
}

// create salt data set by solving ideal astfem equations
US_LammAstfvm::CosedData::CosedData(const US_Model& amodel, US_SimulationParameters asparms, US_DataIO::RawData *asim_data,
                                    QList<US_CosedComponent>* cosed_comps, double &density, double &viscosity) {
   dbg_level = US_Settings::us_debug();
   model = amodel;
   simparms = asparms;
   sa_data = *asim_data;
   cosed_components = cosed_comps;
   count = 0;
   DbgLv(2) << "SaltD: sa_data avg.temp." << sa_data.average_temperature();
   DbgLv(2) << "SaltD: asim_data avg.temp." << asim_data->average_temperature();

   Nt = sa_data.scanCount();
   Nx = sa_data.pointCount();

   model.components.resize(1);
   // use salt's molar concentration, if possible, for initial concentration
   double conc0 = model.components[ 0 ].molar_concentration;
   conc0 = (conc0 < 0.01) ? model.components[ 0 ].signal_concentration : conc0;
   conc0 = (conc0 < 0.01) ? 2.5 : conc0;
//conc0=3100.0;
   model.components[ 0 ].signal_concentration = conc0;

   simparms.meshType = US_SimulationParameters::ASTFEM;
   simparms.gridType = US_SimulationParameters::MOVING;
//   if ( !simparms.band_forming )simparms.band_volume = 0.0;
//   simparms.band_forming = true;
   DbgLv(1) << "simparms.radial_res" << simparms.radial_resolution << "new" << ( sa_data.radius( Nx - 1 ) - sa_data.radius( 0 ) ) / (double)( Nx - 1 );
   simparms.radial_resolution =
      ( sa_data.radius( Nx - 1 ) - sa_data.radius( 0 ) ) / (double)( Nx - 1 );
   simparms.firstScanIsConcentration = false;
   DbgLv(2) << "SaltD: Nx" << Nx << "r0 rn ri" << sa_data.radius(0) << sa_data.radius(Nx - 1)
            << simparms.radial_resolution;

   auto *astfem = new US_Astfem_RSA(model, simparms);

   //astfem->setTimeInterpolation( true );
   astfem->setTimeInterpolation(false);
   astfem->set_debug_flag(dbg_level);
   US_Buffer buffer = US_Buffer();
   buffer.viscosity = viscosity;
   buffer.density = density;
   buffer.manual = true;
   buffer.description = QString("Cosed Buffer Base");
   visc = viscosity;
   dens = density;
   astfem->set_buffer(buffer);

   for ( int ii = 0; ii < Nt; ii++ )
      for ( int jj = 0; jj < Nx; jj++ )
         sa_data.setValue(ii, jj, 0.0);

   DbgLv(2) << "SaltD: model comps" << model.components.size();
   DbgLv(2) << "SaltD: amodel comps" << amodel.components.size();
   DbgLv(2) << "SaltD: comp0 s d s_conc" << model.components[ 0 ].s << model.components[ 0 ].D
            << model.components[ 0 ].signal_concentration;
   DbgLv(2) << "SaltD:fem: m b  s D  rpm" << simparms.meniscus << simparms.bottom << model.components[ 0 ].s
            << model.components[ 0 ].D << simparms.speed_step[ 0 ].rotorspeed;
   DbgLv(2) << "SaltD: (0)Nt Nx" << Nt << Nx << "temperature" << sa_data.scanData[ 0 ].temperature;
   if ( dbg_level > 0 ) {
      DbgLv(1) << "SaltD: model";
      model.debug();
      DbgLv(1) << "SaltD: simparms:";
      simparms.debug();
      DbgLv(1) << "SaltD: sa_data:"<< sa_data.scanCount() << sa_data.pointCount();
   }

   astfem->set_simout_flag(false);         // set flag to output raw simulation
   DbgLv(2) << "SaltD: (2)Nx" << Nx << "r0 r1 rm rn" << sa_data.radius(0) << sa_data.radius(1) << sa_data.radius(Nx - 2)
            << sa_data.radius(Nx - 1);

   astfem->calculate(sa_data, *cosed_components);            // solve equations to create data
   DbgLv(1) << "SaltD: sa_data:"<< sa_data.scanCount() << sa_data.pointCount();
   Nt = sa_data.scanCount();
   Nx = sa_data.pointCount();

   DbgLv(2) << "SaltD: (3)Nx" << Nx << "r0 r1 rm rn" << sa_data.radius(0) << sa_data.radius(1) << sa_data.radius(Nx - 2)
            << sa_data.radius(Nx - 1);
   DbgLv(2) << "SaltD: Nt Nx" << Nt << Nx << "temp" << sa_data.scanData[ 0 ].temperature;
   DbgLv(2) << "SaltD: sa sc0 sec omg" << sa_data.scanData[ 0 ].seconds << sa_data.scanData[ 0 ].omega2t;
   // Limit salt amplitudes to reasonable values
   const double maxsalt = 1e100;
   const double minsalt = -9e99;
   const double minsala = 1e-100;
   int nchg = 0;
   for ( int ii = 0; ii < Nt; ii++ ) {
      for ( int jj = 0; jj < Nx; jj++ ) {
         double saltv = sa_data.value(ii, jj);
         double salta = qAbs(saltv);
         if ( ii < 2 && (jj + 3) > Nx )
            DbgLv(2) << "SaltD:  ii jj" << ii << jj << "saltv salta" << saltv << salta;

         if ( saltv != saltv ) {  // Nan!!!
            nchg++;
            if ( jj > 0 ) {
               saltv = sa_data.value(ii, jj - 1);
            } else {
               saltv = saltv > 0 ? maxsalt : minsalt;
            }
            sa_data.setValue(ii, jj, saltv);
            DbgLv(2) << "SaltD:  salt *NaN* ii jj adj-saltv" << ii << jj << saltv;
         } else if ( salta > maxsalt || saltv < minsalt ) {  // Amplitude too large
            nchg++;
            salta = qMin(maxsalt, qMax(minsalt, salta));
            saltv = saltv > 0 ? salta : -salta;
            sa_data.setValue(ii, jj, saltv);
         } else if ( salta < minsala ) {  // Amplitude too small
            nchg++;
            sa_data.setValue(ii, jj, minsala);
         }
      }
   }
   DbgLv(2) << "SaltD:  salt ampl limit changes" << nchg;

   //xs         = new double [ Nx ];
   //Cs0        = new double [ Nx ];
   //Cs1        = new double [ Nx ];

   if ( dbg_level > 2 ) { // save a copy of the salt data set so that it may be plotted for QC
      QString safile = US_Settings::resultDir() + "/salt_data";
      QDir dir;

      dir.mkpath(safile);
      safile = safile + "/salt_data.RA.1.S.260.auc";
      US_DataIO::writeRawData(safile, sa_data);
   }

   delete astfem;                           // astfem solver no longer needed
   cosed_comp_data[ model.components[ 0 ].analyteGUID ] = sa_data;
   is_empty = false;
}

void US_LammAstfvm::CosedData::initCosed(double* Cs0, double* Cs1, double* xs) {
   double t0 = sa_data.scanData[ 0 ].seconds; // times of 1st 2 salt scans
   double t1 = sa_data.scanData[ 1 ].seconds;
   int scn = 2;                             // index to next scan to use
   Nt = sa_data.scanCount() - 2;       // scan count less two used here

   for ( int j = 0; j < Nx; j++ ) {  // get 1st two salt arrays from 1st two salt scans
      Cs0[ j ] = sa_data.value(0, j);
      Cs1[ j ] = sa_data.value(1, j);
      xs[ j ] = sa_data.radius(j);
   }
   int k = Nx / 2;
   int n = Nx - 1;
   DbgLv(2) << "initSalt: t0 t1 Nt" << t0 << t1 << Nt;
   DbgLv(2) << "initSalt:  xs Cs0 Cs1 j" << xs[ 0 ] << Cs0[ 0 ] << Cs1[ 0 ] << 0;
   DbgLv(2) << "initSalt:  xs Cs0 Cs1 j" << xs[ k ] << Cs0[ k ] << Cs1[ k ] << k;
   DbgLv(2) << "initSalt:  xs Cs0 Cs1 j" << xs[ n ] << Cs0[ n ] << Cs1[ n ] << n;
}

void US_LammAstfvm::CosedData::InterpolateCCosed(int N, const double *x, double t, double *DensCosed, double *ViscCosed) {
   count = (count + 1) % 20;
   double t0 = sa_data.scanData[ 0 ].seconds; // times of 1st 2 salt scans
   double t1 = sa_data.scanData[ 1 ].seconds;
   int scn = 2;                             // index to next scan to use
   int Nt_cosed = sa_data.scanCount() - 3;       // scan count less two used here
   double* Cs0;      // density for the 1st time interval
   double* Cs1;      // density for the 2nd time interval
   double* xs;
   QVector< double > tmpCs0;
   QVector< double > tmpCs1;
   QVector< double > tmpxs;
   tmpCs0.fill(0.0, N);
   tmpCs1.fill(0.0, N);
   tmpxs.fill(0.0, N);
   Cs0 = tmpCs0.data();
   Cs1 = tmpCs1.data();

   xs = tmpxs.data();
   foreach (US_Model::SimulationComponent comp, model.components) {
      sa_data = cosed_comp_data[ comp.analyteGUID ];
      initCosed(Cs0,Cs1,xs);
      DbgLv(2) << "SaltD:ntrp:  N t t1 Nt Nx" << N << t << t1 << Nt_cosed << Nx << "Cs0N Cs1N" << Cs0[ Nx - 1 ]
               << Cs1[ Nx - 1 ];

      while ((t1 < t) && (scn < sa_data.scanCount())) {  // walk down salt scans until we are straddling desired time value
         t0 = t1;
         t1 = sa_data.scanData[ scn ].seconds;// Nt = time level left
         Nt_cosed--;
         scn++;
         DbgLv(3) << "SaltD:ntrp:      0 t 1" << t0 << t << t1 << "  N s" << Nt_cosed << scn;
      }
      DbgLv(2) << "SaltD:ntrp:   t0 t t1" << t0 << t << t1 << "  Nt scn" << Nt_cosed << scn;
      for ( int j = 0; j < Nx; j++ ) {  // get 1st two salt arrays from 1st two salt scans
         Cs0[ j ] = sa_data.value(scn-2, j);
         Cs1[ j ] = sa_data.value(scn-1, j);
      }
      // interpolate between t0 and t1
      double et1 = (t - t0) / (t1 - t0);
      if (et1 < 0.0 || et1 > 1.0){
         DbgLv(2) << "test";
      }
      et1 = (et1 > 1.0) ? 1.0 : et1;
      et1 = (et1 < 0.0) ? 0.0 : et1;
      double et0 = 1.0 - et1;

      // interpolate between xs[k-1] and xs[k]
      int k = 1;
      int Lx = Nx;
      int knan = 0;
      double dens_coeff[6] = {0.0};               //!< The density coefficients.
      double visc_coeff[6] = {0.0};
      foreach (US_CosedComponent i, *cosed_components) {
         if ( i.GUID == comp.analyteGUID || i.name == comp.name ) {
            for ( int iii = 0; iii < 6; iii++ ) {
               dens_coeff[ iii ] = i.dens_coeff[ iii ];
               visc_coeff[ iii ] = i.visc_coeff[ iii ];
            }
            DbgLv(2) << "found cosed comp to analyte";
            break;
         }
      }
      for ( int jf = 0; jf < N; jf++ )      // loop for all x[m]
      {
         double xj = x[ jf ];
         while ( xj > xs[ k ] && k < Lx ) k++; // radial point

         // linear interpolation
         int m = k - 1;
         if ((xs[k]<xs[m] || xs[k] < xj || xs[m] > xj) && k == Lx){
            DbgLv(2)<<"radius corrupted";
         }
         double xik = (xj - xs[ m ]) / (xs[ k ] - xs[ m ]);
         if ((xik < 0.0 || xik > 1.0) && k == Lx){
            DbgLv(2) << "radius corrupted again";
         }
         xik = (xik > 1.0) ? 1.0 : xik;
         xik = (xik < 0.0) ? 0.0 : xik;
         double xim = 1.0 - xik;
         DbgLv(2) << "jf=" << jf << " k=" << k << " m=" << m << " xj=" << xj << " xs[k]" << xs[ k ] << " Lx=" << Lx
                  << " xik=" << xik << " xim=" << xim;
         // interpolate linearly in both time and radius
         double Cm = 0.0;
         Cm = et0 * (xim * Cs0[ m ] + xik * Cs0[ k ]) + et1 * (xim * Cs1[ m ] + xik * Cs1[ k ]);
         //*DEBUG
         if ( Cm != Cm ) {
            knan++;
            if ( knan < 20 )
               DbgLv(2) << "SaltD:ntrp: j,k,xj,xsm,xsk" << jf << k << xj << xs[ m ] << xs[ k ] << "xim xik" << xim
                        << xik << "Csaltj" << Cm;
         }
         //*DEBUG
         double rho;
         double phi;
         double Cmrt = sqrt(Cm);             //  and powers of it
         double Cmsq = Cm * Cm;
         double Cmcu = Cm * Cmsq;
         double Cmqu = Cm * Cmcu;

         rho = Cmrt * dens_coeff[ 1 ] + Cm * dens_coeff[ 2 ] + Cmsq * dens_coeff[ 3 ] + Cmcu * dens_coeff[ 4 ] +
               Cmqu * dens_coeff[ 5 ];

         phi = Cmrt * visc_coeff[ 1 ] + Cm * visc_coeff[ 2 ] + Cmsq * visc_coeff[ 3 ] + Cmcu * visc_coeff[ 4 ] +
                Cmqu * visc_coeff[ 5 ];
         phi = qAbs(phi);
         DensCosed[ jf ] += rho;
         ViscCosed[ jf ] += phi;
         DbgLv(2) << "cosed comp" << comp.name << "Cm"
                  << et0 * (xim * Cs0[ m ] + xik * Cs0[ k ]) + et1 * (xim * Cs1[ m ] + xik * Cs1[ k ]) << rho
                  << phi;
      } // radius loop end
   } // cosed component loop end
}

US_LammAstfvm::CosedData::CosedData() {
   is_empty = true;
   sa_data = US_DataIO::RawData();
   cosed_comp_data.clear();
   cosed_components = nullptr;
   model = US_Model();
   simparms = US_SimulationParameters();
   count = 0;
   Nx = 0;
   Nt = 0;
   dbg_level = 1;
   dens = 0.0;
   visc = 0.0;
}


// construct Lamm solver for finite volume method
US_LammAstfvm::US_LammAstfvm(US_Model &rmodel, US_SimulationParameters &rsimparms, QObject *parent /*=0*/ ) : QObject(
      parent), model(rmodel), simparams(rsimparms) {
   comp_x = 0;           // initial model component index
   cosed_needed = false;
   codiff_needed = false;
   dbg_level = US_Settings::us_debug();
   stopFlag = false;
   movieFlag = true;
   double speed = simparams.speed_step[ 0 ].rotorspeed;
   double *coefs = simparams.rotorcoeffs;
   double stretch = coefs[ 0 ] * speed + coefs[ 1 ] * sq(speed);
   param_m = simparams.meniscus;
   param_b = simparams.bottom;
   param_w2 = sq(speed * M_PI / 30.0);
   DbgLv(2) << "LFvm: m b w2 strtch" << param_m << param_b << param_w2 << stretch;
   param_m -= stretch;
   param_b += stretch;
   DbgLv(2) << "LFvm:  m b" << param_m << param_b;

   MeshSpeedFactor = 1;    // default mesh moving option
   MeshRefineOpt   = 1;    // default mesh refinement option

   NonIdealCaseNo = 0;    // default case: ideal with constant s, D

   sigma = 0.;   // default: s=s_0
   delta = 0.;   // default: D=D_0
   density = DENS_20W;
   compressib = 0.0;
   d_coeff[ 0 ] = DENS_20W;
   v_coeff[ 0 ] = VISC_20W;
   for ( int ii = 1; ii < 6; ii++ ) {
      d_coeff[ ii ] = 0.0;
      v_coeff[ ii ] = 0.0;
   }

   //err_tol         = 1.0e-4;
   err_tol = 1.0e-5;
   // init all variables
   saltdata = nullptr;
   bandFormingGradient = nullptr;
   auc_data = nullptr;
   viscosity = 0.0;
   manual = false;
   vbar_salt = 0.0;
   param_s = 0.0;
   param_D = 0.0;
   dt = 0.0;
}

// primary method to calculate solutions for all species
int US_LammAstfvm::calculate(US_DataIO::RawData &sim_data) {
   auc_data = &sim_data;

   // use given data to create form for internal data; zero initial concs.
   load_mfem_data(sim_data, af_data);

   // set up to report progress to any listener (e.g., us_astfem_sim)
   int nsteps = af_data.scan.size() * model.components.size();

   if ( model.coSedSolute >= 0 ) {  // for co-sedimenting, reduce total steps by one scan
      nsteps -= af_data.scan.size();
   }
#ifndef NO_DB
   emit calc_start(nsteps);
   qApp->processEvents();
#endif

   if (codiff_needed && (bandFormingGradient == nullptr || bandFormingGradient->is_empty || bandFormingGradient->dens_bfg_data.scanCount() == 0)) {
      param_s = model.components.first().s != 0.0 ? model.components.first().s : 1e-14;
      param_D = model.components.first().D;
      dt = log(param_b / param_m) / (param_w2 * param_s * 5000);
      for (US_Model::SimulationComponent &comp: model.components){
         double dt_temp;
         param_s = comp.s != 0.0 ? comp.s : 1e-14;
         param_D = comp.D;
         dt_temp = log(param_b / param_m) / (param_w2 * param_s * 5000);
         int nts = af_data.scan.size();            // nbr. output times (scans)
         double true_dt_min = dt_temp;
         for ( int i = 1; i < nts; i++ ) {
            true_dt_min = min(true_dt_min, af_data.scan[ i ].time - af_data.scan[ i - 1 ].time);
         }
         if ( true_dt_min < dt_temp ) {
            DbgLv(1) << "dt Problem dt=" << dt_temp << " true_dt_min=" << true_dt_min;
            dt_temp = true_dt_min / 1.5;
         }
         dt = min(dt,dt_temp);
      }
      validate_bfg();
      validate_csd();
//       QString visc_data = "visc_data.txt";
//       QString dens_data = "dens_data.txt";
//       {
//           QFile myFile(dens_data);
//           if (!myFile.open(QIODevice::WriteOnly)) {
//               qDebug() << "Could not write to file:" << dens_data << "Error string:" << myFile.errorString();
//               return 0;
//           }
//
//
//           QTextStream out(&myFile);
//           QVector<QString> radius;
//           foreach (double x, bandFormingGradient->dens_bfg_data.xvalues) {
//               radius << QString::number(x, 'f', 4);
//           }
//           out << "radius";
//                   for(int i = 0; i < bandFormingGradient->dens_bfg_data.scanCount(); i++) {
//                   out << "; " << QString::number(i+1, 'f', 0)<< " " <<
//                   QString::number(bandFormingGradient->dens_bfg_data.scanData[i].seconds) << " s";
//               }
//           out << Qt::endl;
//           for (int r = 0; r < radius.count() - 1; r++) {
//               out << radius.value(r);
//                       for (int s = 0; s < bandFormingGradient->dens_bfg_data.scanCount(); s++) {
//                       out << "; " << QString::number(bandFormingGradient->dens_bfg_data.reading(s,r), 'f', 6);
//                   }
//               out << Qt::endl;
//           }
//           myFile.flush();
//           myFile.close();
//       }
//
//       {
//           QFile myFile(visc_data);
//           if (!myFile.open(QIODevice::WriteOnly)) {
//               qDebug() << "Could not write to file:" << visc_data << "Error string:" << myFile.errorString();
//               return 0;
//           }
//
//
//           QTextStream out(&myFile);
//           QVector<QString> radius;
//           foreach (double x, bandFormingGradient->visc_bfg_data.xvalues) {
//                   radius << QString::number(x, 'f', 4);
//               }
//           out << "radius";
//           for(int i = 0; i < bandFormingGradient->visc_bfg_data.scanCount(); i++) {
//               out << "; " << QString::number(i+1, 'f', 0);
//           }
//           out << Qt::endl;
//           for (int r = 0; r < radius.count() - 1; r++) {
//               out << radius.value(r);
//               for (int s = 0; s < bandFormingGradient->visc_bfg_data.scanCount(); s++) {
//                   out << "; " << QString::number(bandFormingGradient->visc_bfg_data.reading(s,r), 'f', 6);
//               }
//               out << Qt::endl;
//           }
//           myFile.flush();
//           myFile.close();
//       }
   }
   // update concentrations for each model component
   for ( int ii = 0; ii < model.components.size(); ii++ ) {
      int rc = solve_component(ii);

      if ( rc != 0 )
         return rc;

   }

#ifndef NO_DB
   emit calc_done();
   qApp->processEvents();
#endif

   // populate user's data set from calculated simulation
   store_mfem_data(sim_data, af_data);

   return 0;
}

// get a solution for a component and update concentrations
int US_LammAstfvm::solve_component(int compx) {
   comp_x = compx;

   param_s20w = model.components[ compx ].s != 0.0 ? model.components[ compx ].s : 1e-14;
   param_D20w = model.components[ compx ].D;
   if ( nonIdealCaseNo() != 0 )            // set non-ideal case number
   {  // if multiple cases, abort
       return 1;
   }
   qDebug() << "LAV:SC: NonIdealCaseNo" << NonIdealCaseNo;
   // make sure the selected model is adjusted for the selected temperature
   // and buffer conditions:
   US_Math2::SolutionData sol_data{};
   sol_data.density = density;
   sol_data.viscosity = viscosity;
   sol_data.vbar20 = model.components[ compx ].vbar20; //The assumption here is that vbar does not change with
   sol_data.vbar = model.components[ compx ].vbar20; //temp, so vbar correction will cancel in s correction
   sol_data.manual = manual;
   US_Math2::data_correction(simparams.temperature, sol_data);
   param_s = param_s20w / sol_data.s20w_correction;
   param_D = param_D20w / sol_data.D20w_correction;
   double t0 = 0.;
   double t1 = 100.;
   double *x0;
   double *u0;
   double *x1;
   double *u1;
   double *u1p0;
   double *u1p;
   double *dtmp;
   double total_t = (param_b - param_m) * 2.0 / (param_s * param_w2 * param_m);
   dt = log(param_b / param_m) / (param_w2 * param_s * 5000);

   int ntcc = (int) (total_t / dt) + 1;      // nbr. times in calculations
   int jt = 0;
   int nts = af_data.scan.size();            // nbr. output times (scans)
   int kt = 0;
   int ncs = af_data.radius.size();          // nbr. concentrations each scan
   int N0;
   int N1;
   int N0u;
   int N1u;
   int istep = compx * nts;

   double true_dt_min = af_data.scan[0].time;
   for ( int i = 1; i < nts; i++ ) {
      true_dt_min = min(true_dt_min, af_data.scan[ i ].time - af_data.scan[ i - 1 ].time);
   }
   if ( true_dt_min < dt ) {
      DbgLv(1) << "dt Problem dt=" << dt << " true_dt_min=" << true_dt_min << "new dt" << true_dt_min / 1.5;
      dt = true_dt_min / 1.5;
   }

   double solut_t = af_data.scan[ nts - 1 ].time;  // true total time
   int ntc = (int) (solut_t / dt) + 1;      // nbr. times in calculations

   QVector<double> conc0;
   QVector<double> conc1;
   QVector<double> rads;

#ifndef NO_DB
   emit comp_progress(compx + 1);
   qApp->processEvents();
#endif
   if ( nonIdealCaseNo() != 0 )            // set non-ideal case number
   {  // if multiple cases, abort
      return 1;
   }
   if ( NonIdealCaseNo == 3 ) {  // compressibility:  8-fold smaller delta-t and greater time points
      dt /= 8.0;
      ntc = (int) (solut_t / dt) + 1;
   }

   DbgLv(1) << "LAsc:  CX=" << comp_x << "  ntcc ntc nts ncs nicase" << ntcc << ntc << nts << ncs << NonIdealCaseNo;
   DbgLv(1) << "LAsc:    tot_t dt sol_t" << total_t << dt << solut_t;
   DbgLv(1) << "LAsc:     m b s w2" << param_m << param_b << param_s << param_s20w << param_D << param_D20w << param_w2;

   conc0.resize(ncs);
   conc1.resize(ncs);
   rads.resize(ncs);

   Mesh *msh = new Mesh(param_m, param_b, simparams.simpoints, 0);

   msh->InitMesh(param_s20w, param_D20w, param_w2);
   int mropt = 0;                   // mesh refine option;
   double dt_old = dt;
   // make settings based on non-ideal case type
   if ( NonIdealCaseNo == 1 )                   // concentration-dependent
   {
      SetNonIdealCase_1(model.components[ comp_x ].sigma, model.components[ comp_x ].delta);
      DbgLv(2) << "LAsc:   sigma delta" << model.components[ comp_x ].sigma << model.components[ comp_x ].delta
               << "  comp_x" << comp_x;
   } else if ( NonIdealCaseNo == 2 )              // co-sedimenting
   {
      SetNonIdealCase_2();
      mropt = 0;

      dt = min(bandFormingGradient->dt,dt);
      solut_t = af_data.scan[ nts - 1 ].time;  // true total time
      ntc = (int) (solut_t / dt) + 1;
   } else if ( NonIdealCaseNo == 3 )              // compressibility
   {
      SetNonIdealCase_3(mropt, err_tol);
   } else if ( NonIdealCaseNo == 4 ) {
      SetNonIdealCase_4();                     // co-diffusing
   } else {
      NonIdealCaseNo = 0;
   }

   SetMeshRefineOpt(mropt);    // mesh refine option
   SetMeshSpeedFactor(1.0);    // mesh speed factor


   // get initial concentration for this component
   double sig_conc = model.components[ comp_x ].signal_concentration;
   DbgLv(1) << "component sig_conc s D" << model.components[ comp_x ].name << sig_conc << model.components[ comp_x ].s
            << model.components[ comp_x ].D;

   QElapsedTimer timer;
   int ktime1 = 0;
   int ktime2 = 0;
   int ktime3 = 0;
   int ktime4 = 0;
   int ktime5 = 0;
   int ktime6 = 0;
   timer.start();
   // initialization
   N0 = msh->Nv;
   N0u = N0 + N0 - 1;
   x0 = new double[N0];
   u0 = new double[N0u];
   N1 = N0;
   N1u = N0u;
   x1 = new double[N1];
   u1 = new double[N1u];

   for ( int jj = 0; jj < N0; jj++ ) {  // initialize X and U values
      int kk = jj + jj;

      x0[ jj ] = msh->x[ jj ];              // r value
      x1[ jj ] = x0[ jj ];
      if ( simparams.band_forming ) {
         // Calculate the width of the lamella
         double angl = simparams.cp_angle != 0.0 ? simparams.cp_angle : 2.5;
         double plen = simparams.cp_pathlen != 0.0 ? simparams.cp_pathlen : 1.2;
         double base = sq(simparams.meniscus) + simparams.band_volume * 360.0 / (angl * plen * M_PI);
         double lamella_width = sqrt(base) - simparams.meniscus;
         if (msh->x[ jj ] < simparams.meniscus + lamella_width){
             u0[kk] = msh->x[ jj ] * sig_conc;
         }
         else
         {
//             base = (msh->x[jj] - (simparams.meniscus + lamella_width)) / lamella_width;
//             u0[kk] = msh->x[ jj ] * sig_conc * exp(-pow(base, 4.0));   // C*r value
             u0[kk] = 0.0;
         }
      } else {
         u0[ kk ] = msh->x[ jj ] * sig_conc;   // C*r value
      }
      u1[ kk ] = u0[ kk ];
   }

   for ( int kk = 1; kk < N0u - 1; kk += 2 ) {  // fill in mid-point U values
      u0[ kk ] = (u0[ kk - 1 ] + u0[ kk + 1 ]) * 0.5;
      u1[ kk ] = u0[ kk ];
   }
   DbgLv(2) << "LAsc:  u0 0,1,2...,N" << u0[ 0 ] << u0[ 1 ] << u0[ 2 ] << u0[ N0u - 3 ] << u0[ N0u - 2 ]
            << u0[ N0u - 1 ];

   for ( int jj = 0; jj < ncs; jj++ ) {  // get output radius vector
      rads[ jj ] = af_data.radius[ jj ];
   }
   DbgLv(2) << "LAsc:  r0 rn ncs rsiz" << rads[ 0 ] << rads[ ncs - 1 ] << ncs << af_data.radius.size() << rads.size();

   const double u0max = 1e+100;
   const double u0min = -u0max;

   for ( int jj = 0; jj < N0u; jj++ ) {
      u0[ jj ] = qMin(u0max, qMax(u0min, u0[ jj ]));
      u1[ jj ] = qMin(u0max, qMax(u0min, u1[ jj ]));
   }
   DbgLv(2) << "LAsc:   u0 0,1,2...,N" << u0[ 0 ] << u0[ 1 ] << u0[ 2 ] << u0[ N0u - 3 ] << u0[ N0u - 2 ]
            << u0[ N0u - 1 ];

#ifndef NO_DB
   int ktinc = 5;                        // signal progress every 5th scan
   if ( nts > 10000 )
      ktinc = qMax(2, ((nts + 5000) / 10000));
   if ( nts < 100 )
      ktinc = 1;
   DbgLv(2) << "LAsc:   nts ktinc" << nts << ktinc;
#endif
   double ts;
   double u_ttl;


   QDir dir;
   QString tmpDir = US_Settings::tmpDir();
   if ( !dir.exists(tmpDir)) dir.mkpath(tmpDir);

   QFile ftto(tmpDir + "/tt0-ufvm");
   if ( dbg_level > 0 ) {
      if ( !ftto.open(QIODevice::WriteOnly | QIODevice::Truncate))
         DbgLv(1) << "*ERROR* Unable to open tt0-ufvm";
   }
   QTextStream tso(&ftto);
//ntc=ntcc;
   if ( dbg_level > 0 )
      tso << ntc << "\n";
   double runtime = 0.0;
   double dt_scaling = 0.0;
   bool break_switch = false;
   const double original_dt = dt;
   // main loop for time
   for ( jt = 0, kt = 0; jt < ntc; jt++ ) {
      DbgLv(1) << "---------------------------------------";
      timer.restart();
      if (NonIdealCaseNo == 2){
          if (t0 > af_data.scan.last().time && !break_switch){
              break_switch = true;
          }
          else if (t0 > af_data.scan.last().time && break_switch){
              break;
          }
          else if (runtime > 5000){
              dt_scaling *= 1.05;
          }
          else if (runtime > 4000 ){
              dt_scaling *= 1.01;
          }
          else if (runtime > 3600){
              dt_scaling *= 1.005;
          }
          else if (runtime > 1800){
              dt_scaling *= 1.001;
          }
          else if (runtime > 1000){
              dt_scaling += original_dt*0.1;
          }else if (runtime > 300){
              dt_scaling += original_dt*0.05;
          }
          t0 = runtime;
          runtime += qMin((original_dt + dt_scaling),dt_old);
          t1 = runtime;
          dt = t1-t0;
      }
      else{
          t0 = dt * (double) jt;
          t1 = t0 + dt;
      }
      ts = af_data.scan[ kt ].time;           // time at output scan
      while ( ts < t0 && kt < af_data.scan.size() - 1) {
         int kt_old = kt;
         double ts_old = ts;
         double tmp = af_data.scan[ kt + 1 ].time;
         DbgLv(2) << "LAsc: kt increase kt_old=" << kt_old << "ts_old=" << ts_old << "tmp=" << tmp << "t1=" << t1;
         if ( tmp > t1 ) {
            DbgLv(2) << "LAsc: new ts would exceed t1, escape";
            break;
         }
         kt++;
         ts = tmp;
      }
      N0u = N0 + N0 - 1;
      DbgLv(1) << "LAsc: MainLoop Time jt=" << jt << "kt=" << kt << "t0=" << t0 << "t1=" << t1 << "ts=" << ts << "N0="
               << N0 << "N0u=" << N0u;
      if ( dbg_level > 0 && ((jt / 10) * 10) == jt ) {
         u_ttl = IntQs(x0, u0, 0, -1, N0 - 2, 1);
         DbgLv(2) << "LAsc:    jt,kt,t0=" << jt << kt << t0 << " Nv=" << N0 << "u_ttl=" << u_ttl;
         DbgLv(2) << "LAsc:  u0 0,1,2...,N" << u0[ 0 ] << u0[ 1 ] << u0[ 2 ] << u0[ N0u - 3 ] << u0[ N0u - 2 ]
                  << u0[ N0u - 1 ];
         tso << QString("%1 %2 %3\n").arg(QString::number(t0,'f', 5), QString::number(N0), QString::number(u_ttl,'f', 5));
         for ( int j = 0; j < N0; j++ )
            tso << QString("%1\n").arg( x0[ j ],0,'g',6);
         tso << QString("\n");
         for ( int j = 0; j < N0u; j++ )
            tso << QString("%1\n").arg(u0[ j ],0,'g',7);
         tso << QString("\n\n");
      }
      ktime1 += (int) timer.restart();

      u1p0 = new double[N0u];
      LammStepSedDiff_P(t0, dt, N0 - 1, x0, u0, u1p0);
//      for (int i = 0; i < N0u; i++){
//          double v = u1p0[i];
//          if (qAbs(v)< 1e-6){
//              u1p0[i] = 0.0;
//          }
//      }
      ktime2 += (int) timer.restart();

      if ( MeshRefineOpt == 1 ) {

         msh->RefineMesh(u0, u1p0, err_tol);
         ktime3 += (int) timer.restart();

         N1 = msh->Nv;
         N1u = N1 + N1 - 1;
         u1p = new double[N1u];

         delete[] x1;
         x1 = new double[N1];

         for ( int jj = 0; jj < N1; jj++ )
            x1[ jj ] = msh->x[ jj ];

         ProjectQ(N0 - 1, x0, u1p0, N1 - 1, x1, u1p);
//          for (int i = 0; i < N1u; i++){
//              double v = u1p[i];
//              if (qAbs(v)< 1e-6){
//                  u1p[i] = 0.0;
//              }
//          }
         delete[] u1;
         u1 = new double[N1u];

         ktime4 += (int) timer.restart();
         LammStepSedDiff_C(t0, dt, N0 - 1, x0, u0, N1 - 1, x1, u1p, u1);

         delete[] u1p;
      } else {
         ktime4 += (int) timer.restart();
         LammStepSedDiff_C(t0, dt, N0 - 1, x0, u0, N1 - 1, x1, u1p0, u1);
      }
//       for (int i = 0; i < N1u; i++){
//           double v = u1[i];
//           if (qAbs(v)< 1e-6){
//               u1[i] = 0.0;
//           }
//       }
      ktime5 += (int) timer.restart();
      // see if current scan is between calculated times; output scan if so

      if ( ts >= t0  &&  ts <= t1 )
      {  // interpolate concentrations quadratically in x; linearly in time
         double f0 = ( t1 - ts ) / ( t1 - t0 );       // fraction of conc0
         double f1 = ( ts - t0 ) / ( t1 - t0 );       // fraction of conc1
         DbgLv(2) << "LAsc: call qI  t0 ts t1" << t0 << ts << t1;
         // do quadratic interpolation to fill out concentrations at time t0
         quadInterpolate(x0, u0, N0, rads, conc0);

         // do quadratic interpolation to fill out concentrations at time t1
         quadInterpolate(x1, u1, N1, rads, conc1);
         DbgLv(2) << "LAsc:  N0 N1 N0u N1u ncs" << N0 << N1 << N0u << N1u << ncs;
         DbgLv(2) << "LAsc:  x0[0] x0[H] x0[N]" << x0[ 0 ] << x0[ N0 / 2 ] << x0[ N0 - 1 ];
         DbgLv(2) << "LAsc:  x1[0] x1[H] x1[N]" << x1[ 0 ] << x1[ N1 / 2 ] << x1[ N1 - 1 ];
         DbgLv(2) << "LAsc:   r[0]  r[H]  r[N]" << rads[ 0 ] << rads[ ncs / 2 ] << rads[ ncs - 1 ];
         DbgLv(2) << "LAsc:  u0[0] u0[H] u0[N]" << u0[ 0 ] << u0[ N0u / 2 ] << u0[ N0u - 1 ];
         DbgLv(2) << "LAsc:  u1[0] u1[H] u1[N]" << u1[ 0 ] << u1[ N1u / 2 ] << u1[ N1u - 1 ];
         DbgLv(2) << "LAsc:  c0[0] c0[H] c0[N]" << conc0[ 0 ] << conc0[ ncs / 2 ] << conc0[ ncs - 1 ];
         DbgLv(2) << "LAsc:  c1[0] c1[H] c1[N]" << conc1[ 0 ] << conc1[ ncs / 2 ] << conc1[ ncs - 1 ];
         double utt0 = IntQs(x0, u0, 0, -1, N0 - 2, 1);
         double utt1 = IntQs(x1, u1, 0, -1, N1 - 2, 1);
         DbgLv(2) << "LAsc:   utt0 utt1" << utt0 << utt1;
         DbgLv(2) << "LAsc:stopFlag" << stopFlag;

         double cmax = 0.0;
         double rmax = 0.0;

//f0=1.0; f1=0.0;
         for ( int jj = 0; jj < ncs; jj++ ) {  // update concentration vector with linear interpolation for time
            af_data.scan[ kt ].conc[ jj ] += (conc0[ jj ] * f0 + conc1[ jj ] * f1);
            double Cm = af_data.scan[ kt ].conc[ jj ];
            if ( Cm > cmax ) {
               cmax = Cm;
               rmax = af_data.radius[ jj ];
            }
         }
         DbgLv(2) << "LAsc: t=" << ts << "Cmax=" << cmax << " r=" << rmax;
         DbgLv(2) << "LAsc:   co[0] co[H] co[N]  kt" << af_data.scan[ kt ].conc[ 0 ]
                  << af_data.scan[ kt ].conc[ ncs / 2 ] << af_data.scan[ kt ].conc[ ncs - 1 ] << kt;

         istep++;  // bump progress step

#ifndef NO_DB
         if (((kt / ktinc) * ktinc) == kt ||
             (kt + 1) == nts ) {  // signal progress at every "ktinc'th" scan or final one
            emit calc_progress(istep);
            DbgLv(2) << "LAsc: istep" << istep;
            qApp->processEvents();
         }
#endif
#ifndef NO_DB
//      if ( movieFlag  &&
//         ( ( ( kt / ktinc ) * ktinc ) == kt  ||  ( kt + 1 ) == nts ) )
         if ( movieFlag ) {
            emit new_scan(&af_data.radius, af_data.scan[ kt ].conc.data());
            emit new_time(af_data.scan[ kt ].time);
            emit current_speed((int) af_data.scan[ kt ].rpm);
            qApp->processEvents();
         }
         else
#endif
          {
              emit new_time(af_data.scan[kt].time);
              emit current_speed((int) af_data.scan[kt].rpm);
              qApp->processEvents();
          }
         kt++;    // bump output time(scan) index


      if ( stopFlag ) break;
      } else {
         DbgLv(1) << "kt not updated";
      }

      delete[] u1p0;

      if ( kt >= nts )
         break;   // if all scans updated, we are done

      qApp->processEvents();
      if ( stopFlag ) break;
      // switch x,u arrays for next iteration
      N0 = N1;
      N0u = N1u;
      dtmp = x0;
      x0 = x1;
      x1 = dtmp;
      dtmp = u0;
      u0 = u1;
      u1 = dtmp;
   } // end main time loop

   if ( dbg_level > 0 )
      ftto.close();

   if ( dbg_level > 0 ) {
      DbgLv(1) << "#####################################";
      DbgLv(1) << "component: " << model.components[ compx ].name;
      // calculate and print the integral of scan curves
      double cimn = 9e+14;
      double cimx = 0.0;
      double ciav = 0.0;
      double dltr = (af_data.radius[ 1 ] - af_data.radius[ 0 ]) * 0.5;

      for ( int ii = 0; ii < af_data.scan.size(); ii++ ) {
         double csum = 0.0;
         double pval = af_data.scan[ ii ].conc[ 0 ];

         for ( int jj = 1; jj < af_data.scan[ ii ].conc.size(); jj++ ) {
            double cval = af_data.scan[ ii ].conc[ jj ];
            csum += ( ( cval + pval ) * 0.5 * (sq(af_data.radius[jj])-sq(af_data.radius[jj-1])));
            pval = cval;
//if ( ii < 19  &&  ( (jj/100)*100 == jj || (jj+5)>nconc ) )
// DbgLv(3) << "   jj cval dltr csum" << jj << cval << dltr << csum;
         }
         DbgLv(1) << "Scan" << ii + 1 << " Time " << af_data.scan[ ii ].time << "  Integral" << csum;
         cimn = (cimn < csum) ? cimn : csum;
         cimx = (cimx > csum) ? cimx : csum;
         ciav += csum;
      }

      ciav /= (double) af_data.scan.size();
      double cidf = cimx - cimn;
      double cidp = (double) (qRound(10000.0 * cidf / ciav)) / 100.0;
      DbgLv(1) << "  Integral Min Max Mean" << cimn << cimx << ciav;
      DbgLv(1) << "  ( range of" << cidf << "=" << cidp << " percent of mean )";
   }

   delete[] x0;  // clean up
   delete[] u0;
   delete[] x1;
   delete[] u1;
   delete msh;
   ktime6 += (int) timer.elapsed();
   DbgLv(2) << "compx" << comp_x << "times 1-6" << ktime1 << ktime2 << ktime3 << ktime4 << ktime5 << ktime6;
   return 0;
}

void US_LammAstfvm::set_buffer(US_Buffer buffer, US_Math_BF::Band_Forming_Gradient* bfg, CosedData* csD) {
   DbgLv(0) << "ASTFVM:set_buffer";
   density = buffer.density; // for compressibility
   viscosity = buffer.viscosity;
   compressib = buffer.compressibility;
   manual = buffer.manual;

   buffer.compositeCoeffs(d_coeff, v_coeff);
   if ( !buffer.cosed_component.isEmpty()) {
      cosed_components = buffer.cosed_component;
       if (csD == nullptr){
           DbgLv(1) << "csD: nothing to do";
           cosed_comp_data.clear();
       }
       else if (!csD->is_empty && csD->cosed_components != nullptr){
           DbgLv(1) << "reused old csD";
           saltdata = csD;
         cosed_comp_data.clear();
         cosed_comp_data = saltdata->cosed_comp_data;
         cosed_comp_data.detach();
         if (!csD->cosed_comp_data.isEmpty()){
            cosed_needed = true;
         }
       }
       else{
           DbgLv(1) << "csD is not null pointer but empty";
           cosed_comp_data.clear();
       }
      SetNonIdealCase_2();
      DbgLv(1) << "ASTFVM:set_buffer:" << codiff_needed;
      DbgLv(1) << "ASTFVM:set_buffer:" << bfg;
      if (codiff_needed){
      if (bfg == nullptr){
         DbgLv(1) << "no bfg, calc new";
         bandFormingGradient  = new US_Math_BF::Band_Forming_Gradient( simparams.meniscus, simparams.bottom,
                                                                       simparams.band_volume,
                                                                       cosed_components, simparams.cp_pathlen,
                                                                       simparams.cp_angle);
         if (!bandFormingGradient->upper_comps.isEmpty()){
            bandFormingGradient->get_eigenvalues();
         }
      }
      else if (!bfg->is_empty){
         DbgLv(1) << "reused old bfg";
         bandFormingGradient = bfg;
         codiff_needed = true;
      }
      else{
         DbgLv(1) << "bfg, but empty";
      bandFormingGradient  = new US_Math_BF::Band_Forming_Gradient( simparams.meniscus, simparams.bottom,
                                                                    simparams.band_volume,
                                                                    cosed_components, simparams.cp_pathlen,
                                                                    simparams.cp_angle);
         if (!bandFormingGradient->upper_comps.isEmpty()){
            bandFormingGradient->get_eigenvalues();
            codiff_needed = true;
         }

      }
      }
      else{bandFormingGradient  = new US_Math_BF::Band_Forming_Gradient();}
      DbgLv(0) << "buff cosed bfg: beta count" << bandFormingGradient->eigenvalues.count();
   }
}

void US_LammAstfvm::SetNonIdealCase_1(double sigma_k, double delta_k) {
   sigma = sigma_k;                   // for concentration dependency
   delta = delta_k;
}

void US_LammAstfvm::SetNonIdealCase_2() {
   DbgLv(1) << "SetNonIdealCase_2";
   US_Model cosed_model = model;
   cosed_model.coSedSolute = -1;
   cosed_model.components.clear();
   US_Model cosed_model_tmp = model;
   cosed_model_tmp.coSedSolute = -1;
   double base_density = 0.0;
   double base_viscosity = 0.0;
   QMap<QString, US_CosedComponent> upper_cosed;
   QMap<QString, US_CosedComponent> lower_cosed;
   QList<QString> base_comps;
   foreach (US_CosedComponent i, cosed_components) {
      DbgLv(1) << "buff dens_coeff" << i.dens_coeff[ 0 ] << i.dens_coeff[ 1 ] << i.dens_coeff[ 2 ]
               << i.dens_coeff[ 3 ] << i.dens_coeff[ 4 ] << i.dens_coeff[ 5 ];
      DbgLv(1) << "buff visc_coeff" << i.visc_coeff[ 0 ] << i.visc_coeff[ 1 ] << i.visc_coeff[ 2 ]
               << i.visc_coeff[ 3 ] << i.visc_coeff[ 4 ] << i.visc_coeff[ 5 ];
      if ( !i.overlaying && upper_cosed.contains(i.name)) {
         // the current component is in the lower part, but there is another component with the same name in the
         // overlaying section of the band forming gradient
         US_CosedComponent j = upper_cosed[ i.name ];
         if ( j.conc > i.conc ) {
            // the concentration is higher in upper part, move it completely to the upper part and set the
            // concentration to the excess concentration
            j.conc = j.conc - i.conc;
            upper_cosed[ j.name ] = j;
            continue;
         } else if ( fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON ) {
            // the concentration of both components is roughly equal, remove the component from the upper and lower part
            upper_cosed.remove(j.name);
            continue;
         } else {
            j.conc = i.conc - j.conc;
            lower_cosed[ j.name ] = j;
            upper_cosed.remove(j.name);
            continue;
         }
      }
      if ( i.overlaying && lower_cosed.contains(i.name)) {
         // the current component is in the lower part, but there is another component with the same name in the
         // overlaying section of the band forming gradient
         US_CosedComponent j = lower_cosed[ i.name ];
         if ( j.conc > i.conc ) {
            // the concentration is higher in lower part, move it completely to the lower part and set the
            // concentration to the excess concentration
            j.conc = j.conc - i.conc;
            lower_cosed[ j.name ] = j;
            continue;
         } else if ( fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON ) {
            // the concentration of both components is roughly equal, remove the component from the upper and lower part
            lower_cosed.remove(j.name);
            continue;
         } else {
            j.conc = i.conc - j.conc;
            upper_cosed[ j.name ] = j;
            lower_cosed.remove(j.name);
            continue;
         }
      }
      if ( i.overlaying )
         upper_cosed[ i.name ] = i;
      else
         lower_cosed[ i.name ] = i;

   }
   // Determine the base of the buffer
   foreach (US_CosedComponent cosed_comp, cosed_components) {
      if ( cosed_comp.overlaying ) { continue; } // overlaying components can't be part of the base of the buffer
      if ( lower_cosed.contains(cosed_comp.name) &&
           (fabs(lower_cosed[ cosed_comp.name ].conc - cosed_comp.conc) < GSL_ROOT5_DBL_EPSILON) &&
           cosed_comp.s_coeff == 0.0 && cosed_comp.d_coeff == 0.0 ) {
         // the concentration matches the original one entered. -> part of the buffer base
         base_comps << cosed_comp.GUID + cosed_comp.componentID;
         base_density += cosed_comp.dens_coeff[ 0 ];
         base_viscosity += cosed_comp.visc_coeff[ 0 ];
      }
   }
   // make sure the selected model is adjusted for the selected temperature
   // and buffer conditions:
   US_Math2::SolutionData sol_data{};
   sol_data.density = base_density;
   sol_data.viscosity = base_viscosity;
   sol_data.manual = true;
   foreach(US_CosedComponent cosed_comp, cosed_components) {
      // get the excess concentrations
      if ( cosed_comp.overlaying && upper_cosed.contains(cosed_comp.name)) {
         cosed_comp = upper_cosed.value(cosed_comp.name);
      } else if ( !cosed_comp.overlaying && lower_cosed.contains(cosed_comp.name)) {
         cosed_comp = lower_cosed.value(cosed_comp.name);
      } else {
         DbgLv(1) << "nothing";
         continue;
      }
      if ( cosed_comp.s_coeff == 0.0 && cosed_comp.d_coeff == 0.0 ) {
         DbgLv(1) << "not cosedimenting";
         continue;
      }
      if (cosed_comp.s_coeff == 0.0){
         DbgLv(1) << "pure diffusive";
         codiff_needed = true;
         continue;
      }
      cosed_needed = true;
      if (auc_data == nullptr){
         continue;
      }
      cosed_model_tmp.components.clear();
      US_Model::SimulationComponent tmp = US_Model::SimulationComponent();
      tmp.name = cosed_comp.name;
      tmp.analyteGUID = cosed_comp.GUID;
      tmp.molar_concentration = cosed_comp.conc;
      tmp.signal_concentration = cosed_comp.conc;
      tmp.vbar20 = cosed_comp.vbar;
      if (cosed_comp_data.contains(tmp.analyteGUID)){
         continue;}
      sol_data.vbar20 = cosed_comp.vbar; //The assumption here is that vbar does not change with
      sol_data.vbar = cosed_comp.vbar; //temp, so vbar correction will cancel in s correction
      US_Math2::data_correction(simparams.temperature, sol_data);
      tmp.s = cosed_comp.s_coeff / sol_data.s20w_correction;
      tmp.D = cosed_comp.d_coeff / sol_data.D20w_correction;
      tmp.f_f0 = 0.0;
      tmp.analyte_type = 4;
      cosed_model.components << tmp;
      cosed_model_tmp.components << tmp;
      cosed_model_tmp.update_coefficients();
         DbgLv(1) << "NonIdeal2: calc saltdata";
      saltdata = new CosedData(cosed_model_tmp, simparams, auc_data, &cosed_components, base_density, base_viscosity);
      cosed_comp_data[ tmp.analyteGUID ] = saltdata->sa_data;
      DbgLv(1) << "NonIdeal2: create saltdata";
      cosed_model.update_coefficients();
      saltdata->model = cosed_model;
      saltdata->cosed_comp_data = cosed_comp_data;
      saltdata->cosed_comp_data.detach();
      DbgLv(1) << "CosedData: cosed_model comp" << saltdata->model.components.size() << "cosed_comp_data"
               << cosed_comp_data.size() << "sa_data.scanCount()" << saltdata->sa_data.scanCount();
      DbgLv(2) << "NonIdeal2: initSalt  comp_x" << comp_x;
   }
   DbgLv(1) << "NonIdeal2: prep saltdata" << cosed_needed << codiff_needed;
   if (!cosed_comp_data.isEmpty()){
   saltdata->sa_data = cosed_comp_data.first();}
   DbgLv(1) << "NonIdeal2: finished";
}

void US_LammAstfvm::save_xla(const QString &dirname, US_DataIO::RawData sim_data, int i1) {
   // save co-sedimenting components
   DbgLv(1) << "save_xla_is_called";
//   double mrad      = simparams.meniscus;
//   double brad      = simparams.bottom;
   simparams.bottom = simparams.bottom_position;
   double stretch_value = (simparams.rotorcoeffs[ 0 ] * (double) simparams.speed_step[ i1 ].rotorspeed +
                           simparams.rotorcoeffs[ 1 ] * sq((double) simparams.speed_step[ i1 ].rotorspeed));
   double current_meniscus = simparams.meniscus + stretch_value;
   double current_bottom = simparams.bottom_position + stretch_value;
//   double mrad      = sim_data.xvalues[ 0 ];
//   double brad      = sim_data.xvalues[ sim_data.xvalues.size()-1 ];

   double mrad = current_meniscus;
   double brad = current_bottom;
   double grid_res = simparams.radial_resolution;

   DbgLv(1) << "save_xla_meniscus_bottom" << mrad << brad;

   // Add 30 points in front of meniscus
   int points = sim_data.pointCount();
   points += 31;

   double maxc = 0.0;
   int total_scans = sim_data.scanCount();
   int old_points = sim_data.pointCount();

   for ( int ii = 0; ii < total_scans; ii++ ) {  // Accumulate the maximum computed OD value
      for ( int kk = 0; kk < old_points; kk++ )
         maxc = qMax(maxc, sim_data.value(ii, kk));
   }

   // Compute a data threshold that is scan 1's plateau reading times 3
   QVector<double> xtmpVec(total_scans);
   QVector<double> ytmpVec(total_scans);
   double *xtmp = xtmpVec.data();
   double *ytmp = ytmpVec.data();
   double intercept;
   double slope;
   double sigma2;
   double correl;

   for ( int ii = 0; ii < total_scans; ii++ ) {  // Build time,omega2t points
      xtmp[ ii ] = sim_data.scanData[ ii ].seconds;
      ytmp[ ii ] = sim_data.scanData[ ii ].omega2t;
   }

   // Fit to time,omega2t and use fit to compute the time correction
   US_Math2::linefit(&xtmp, &ytmp, &slope, &intercept, &sigma2, &correl, total_scans);

   double timecorr = -intercept / slope;
   double s20wcorr = -2.0;
   double omega = sim_data.scanData[ 0 ].rpm * M_PI / 30.0;
   double oterm = (sim_data.scanData[ 0 ].seconds - timecorr) * omega * omega * s20wcorr;
   double s1plat = 0.0;
   DbgLv(1) << "Sim:SV: slope intercept timecorr" << slope << intercept << timecorr << "x0 xn y0 yn" << xtmp[ 0 ]
            << xtmp[ total_scans - 1 ] << ytmp[ 0 ] << ytmp[ total_scans - 1 ] << "total_scans" << total_scans;

   for ( int jc = 0; jc < model.components.count(); jc++ ) {
      US_Model::SimulationComponent *sc = &model.components[ jc ];
      double conc = sc->signal_concentration;
      double sval = sc->s;
      s1plat += (conc * exp(oterm * sval));
   }

   double dthresh = maxc;
   s1plat = qMin(s1plat, (dthresh * 0.5));
   DbgLv(1) << "Sim:SV: reset s1plat" << s1plat;


   double maxrad = brad;
   dthresh = maxc;


   points = sim_data.pointCount() + 31;


   QVector<double> tconc_v(points);
   double *temp_conc = tconc_v.data();
   double rad = mrad - 30.0 * grid_res;
   sim_data.xvalues.resize(points);

   for ( int jp = 0; jp < points; jp++ ) {
      sim_data.xvalues[ jp ] = rad;
      rad += grid_res;
   }

   for ( int js = 0; js < total_scans; js++ ) {
      US_DataIO::Scan *scan = &sim_data.scanData[ js ];

      for ( int jp = 30; jp < points-1; jp++ ) {  // Position the computed concentration values after the first 30
         temp_conc[ jp ] = scan->rvalues[ jp - 30 ];
      }

      for ( int jp = 0; jp < 30; jp++ ) {  // Zero the first 30 points
         temp_conc[ jp ] = 0.0;
      }

      temp_conc[ 30 ] = s1plat * 2.0;   // Put a spike at the meniscus

      scan->rvalues.resize(points);

      for ( int jp = 0; jp < points; jp++ ) {  // Store the values: first 30 then computed values
         scan->rvalues[ jp ] = temp_conc[ jp ];
      }
//DbgLv(2) << "WD:sc secs" << scan->seconds;
//if ( ii == 0 || (ii+1) == total_scans ) {
//DbgLv(2) << "WD:S0:c00" << scan->rvalues[0];
//DbgLv(2) << "WD:S0:c01" << scan->rvalues[1];
//DbgLv(2) << "WD:S0:c30" << scan->rvalues[30];
//DbgLv(2) << "WD:S0:cn1" << scan->rvalues[points-2];
//DbgLv(2) << "WD:S0:cnn" << scan->rvalues[points-1]; }
   }

   QString run_id = dirname.section("/", -1, -1);

   DbgLv(1) << "Sim:SV:  run_id_from_save_xla" << run_id << dirname;
   QString stype = QString(QChar(sim_data.type[ 0 ])) + QString(QChar(sim_data.type[ 1 ]));
   QString schann = QString(QChar(sim_data.channel));
   int cell = sim_data.cell;
   int wvlen = qRound(sim_data.scanData[ 0 ].wavelength);
   wvlen = (wvlen < 99) ? 123 : wvlen;
   QString ofname = QString("%1/%2.%3.%4.%5.%6.auc")
         .arg(dirname, run_id, stype, QString(cell), schann, QString(wvlen));

   US_DataIO::writeRawData(ofname, sim_data);
}

void US_LammAstfvm::SetNonIdealCase_3(int &mropt, double &err_toll) {
   mropt = 0;
   err_toll = 1.0e-5;
}

void US_LammAstfvm::SetNonIdealCase_4() {
   DbgLv(2) << "NonIdeal4: called";
   if ( bandFormingGradient == nullptr ) {
      DbgLv(2) << "NonIdeal4: create bandforming gradient";
      bandFormingGradient = new US_Math_BF::Band_Forming_Gradient(simparams.meniscus, simparams.bottom,
                                                                  simparams.band_volume, cosed_components,
                                                                  simparams.cp_pathlen, simparams.cp_angle);
      bandFormingGradient->get_eigenvalues();
   }
}

void US_LammAstfvm::SetMeshSpeedFactor(double speed) {
   MeshSpeedFactor = speed;
}

void US_LammAstfvm::SetMeshRefineOpt(int Opt) {
   MeshRefineOpt = Opt;
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
void US_LammAstfvm::LammStepSedDiff_P(double t, double dt_, int M0, double *x0, double *u0, double *u1) {
   LammStepSedDiff_C(t, dt_, M0, x0, u0, M0, x0, u0, u1);
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
void
US_LammAstfvm::LammStepSedDiff_C(double t, double dt_, int M0, double *x0, double *u0, int M1, const double *x1, const double *u1p,
                                 double *u1) {
   int Ng = 2 * M1;     // number of x_star points
   int *ke = new int[Ng];
   auto *MemDouble = new double[12 * Ng + 15];
   double *flux_p[3];

   double dt2 = dt_ * 0.5;
   double *xt = MemDouble;
   double *xi = xt + Ng;
   double *xg0 = xi + Ng;
   double *xg1 = xg0 + Ng;
   double *ug0 = xg1 + Ng;
   double *ug1 = ug0 + Ng;
   double *Sv = ug1 + Ng;
   double *Dv = Sv + Ng;
   double *flux_u = Dv + Ng;
   flux_p[ 0 ] = flux_u + Ng;
   flux_p[ 1 ] = flux_p[ 0 ] + Ng;
   flux_p[ 2 ] = flux_p[ 1 ] + Ng;
   double *phi = flux_p[ 2 ] + Ng;
   double *phiL = phi + 3;
   double *phiR = phiL + 6;
   QElapsedTimer timer;
   static int ktim1 = 0;
   static int ktim2 = 0;
   static int ktim3 = 0;
   static int ktim4 = 0;
   static int ktim5 = 0;
   static int ktim6 = 0;
   static int ktim7 = 0;
   static int ktim8 = 0;
   timer.start();

   // calculate Sv, Dv at t+dt on xg=(xl, xr)
   for ( int j = 0; j < Ng; j += 2 ) {
      int j2 = j / 2;
      xg1[ j ] = x1[ j2 ] * 0.75 + x1[ j2 + 1 ] * 0.25;    // xl
      xg1[ j + 1 ] = x1[ j2 ] * 0.25 + x1[ j2 + 1 ] * 0.75;    // xr
      ug1[ j ] = (3. * u1p[ j ] + 6. * u1p[ j + 1 ] - u1p[ j + 2 ]) / 8.;
      ug1[ j + 1 ] = (3. * u1p[ j + 2 ] + 6. * u1p[ j + 1 ] - u1p[ j ]) / 8.;
   }
   DbgLv(2) << "preadjust  xg1 0 1 M Nm N" << xg1[ 0 ] << xg1[ 1 ] << xg1[ Ng / 2 ] << xg1[ Ng - 2 ] << xg1[ Ng - 1 ];
   DbgLv(2) << "preadjust  Sv 0 1 M Nm N" << Sv[ 0 ] << Sv[ 1 ] << Sv[ Ng / 2 ] << Sv[ Ng - 2 ] << Sv[ Ng - 1 ];
   AdjustSD(t + dt_, Ng, xg1, ug1, Sv, Dv);
   ktim1 += (int) timer.restart();
   DbgLv(2) << "  xg1 0 1 M Nm N" << xg1[ 0 ] << xg1[ 1 ] << xg1[ Ng / 2 ] << xg1[ Ng - 2 ] << xg1[ Ng - 1 ];
   DbgLv(2) << "  Sv 0 1 M Nm N" << Sv[ 0 ] << Sv[ 1 ] << Sv[ Ng / 2 ] << Sv[ Ng - 2 ] << Sv[ Ng - 1 ];

   // determine xg0=(xls, xrs)
   for ( int j = 0; j < Ng; j++ ) {
      double sw2 = Sv[ j ] * param_w2;
      xg0[ j ] = xg1[ j ] - dt_ * MeshSpeedFactor * sw2 * xg1[ j ] * exp(-qAbs(sw2) * dt_ / 2);

      xg0[ j ] = qMax(param_m, qMin(param_b, xg0[ j ]));

      xt[ j ] = (xg1[ j ] - xg0[ j ]) / dt;
   }
   DbgLv(2) << "  xg0 0 1 M Nm N" << xg0[ 0 ] << xg0[ 1 ] << xg0[ Ng / 2 ] << xg0[ Ng - 2 ] << xg0[ Ng - 1 ] << "Ng"
            << Ng;

   // redistribute xgs so that in between [m,b] and in increasing order
   double bl = param_m;

   for ( int j = 0; j < Ng; j++ ) {
      int cnt = 1;

      while ( xg0[ j ] < bl && (j + 1) < Ng ) {
         j++;
         cnt++;
      }

      double br = qMin(xg0[ j ], param_b);

      for ( int jm = 0; jm < cnt; jm++ ) {
         xg0[ j - jm ] = br - (double) jm / (double) cnt * (br - bl);
         xt[ j - jm ] = (xg1[ j - jm ] - xg0[ j - jm ]) / dt_;
      }

      bl = br;
   }
   DbgLv(2) << "   xg0 0 1 M Nm N" << xg0[ 0 ] << xg0[ 1 ] << xg0[ Ng / 2 ] << xg0[ Ng - 2 ] << xg0[ Ng - 1 ] << "Ng"
            << Ng;
   ktim2 += (int) timer.restart();

   // calculate Flux(phi, t+dt) at all xg1
   fun_dphi(-0.5, phiL);  // basis at xi=-1/2
   fun_dphi(0.5, phiR);

   for ( int j = 0; j < Ng; j++ ) {
      int j2 = j / 2;
      double h = 0.5 * (x1[ j2 + 1 ] - x1[ j2 ]);

      for ( int jm = 0; jm < 3; jm++ )    // at xl
      {
         flux_p[ jm ][ j ] = (xt[ j ] - Sv[ j ] * param_w2 * xg1[ j ] - Dv[ j ] / xg1[ j ]) * phiL[ jm ] +
                             Dv[ j ] * phiL[ jm + 3 ] / h;
      }
      j++;

      for ( int jm = 0; jm < 3; jm++ )     // at xr
      {
         flux_p[ jm ][ j ] = (xt[ j ] - Sv[ j ] * param_w2 * xg1[ j ] - Dv[ j ] / xg1[ j ]) * phiR[ jm ] +
                             Dv[ j ] * phiR[ jm + 3 ] / h;
      }
   }


   // calculate Sv, Dv at (xg0, t)

   LocateStar(M0 + 1, x0, Ng, xg0, ke, xi);    // position of xg0 on mesh x0

   for ( int j = 0; j < Ng; j++ ) {
      fun_phi(xi[ j ], phi);

      int j2 = 2 * ke[ j ];
      ug0[ j ] = u0[ j2 ] * phi[ 0 ] + u0[ j2 + 1 ] * phi[ 1 ] + u0[ j2 + 2 ] * phi[ 2 ];
   }

   // calculate s, D at xg0 on time t
   ktim3 += (int) timer.restart();
   AdjustSD(t, Ng, xg0, ug0, Sv, Dv);
   ktim4 += (int) timer.restart();

   // calculate Flux(u0,t) at all xg0
   // (i) Compute ux at nodes as average of Du from left and right

   auto *ux = new double[M0 + 1];     // D_x(u0) at all x0

   for ( int j = 1; j < M0; j++ )         // internal nodes
   {
      int j2 = 2 * j;
      ux[ j ] = ((u0[ j2 - 2 ] - 4. * u0[ j2 - 1 ] + 3. * u0[ j2 ]) / (x0[ j ] - x0[ j - 1 ]) -
                 (3. * u0[ j2 ] - 4. * u0[ j2 + 1 ] + u0[ j2 + 2 ]) / (x0[ j + 1 ] - x0[ j ])) / 2.;
   }

   int j2 = 2 * M0;
   ux[ 0 ] = -(3. * u0[ 0 ] - 4. * u0[ 1 ] + u0[ 2 ]) / (x0[ 1 ] - x0[ 0 ]) / 2.;
   ux[ M0 ] = (u0[ j2 - 2 ] - 4. * u0[ j2 - 1 ] + 3. * u0[ j2 ]) / (x0[ M0 ] - x0[ M0 - 1 ]) / 2.;

   // (ii) flux(u0,t) at all xg0
   for ( int j = 0; j < Ng; j++ ) {
      double wt = (1. - xi[ j ]) / 2.;
      double uxs = ux[ ke[ j ]] * wt + ux[ ke[ j ] + 1 ] * (1. - wt);    // Du0 at xg0

      if ((xg0[ j ] <= (param_m + 1.e-14)) || (xg0[ j ] >= (param_b - 1.e-14))) {
         flux_u[ j ] = 0.;
      } else {
         flux_u[ j ] =
               -(Sv[ j ] * param_w2 * xg0[ j ] + Dv[ j ] / xg0[ j ]) * ug0[ j ] + Dv[ j ] * uxs + xt[ j ] * ug0[ j ];
      }
   }

   delete[] ux;

   //
   // assemble the linear system of equations
   //
   auto **Mtx = new double *[Ng + 1];
   auto *rhs = new double[Ng + 1];
   for ( int i = 0; i <= Ng; i++ )
      Mtx[ i ] = new double[5];

   ktim5 += (int) timer.restart();
   // Assemble the coefficient matrix
   for ( int i = 1; i < Ng; i += 2 ) {
      int k = (i - 1) / 2;
      double h = 0.5 * (x1[ k + 1 ] - x1[ k ]);
      Mtx[ i ][ 0 ] = 0.;
      Mtx[ i ][ 1 ] = h / 24. + dt2 * (flux_p[ 0 ][ i - 1 ] - flux_p[ 0 ][ i ]);
      Mtx[ i ][ 2 ] = h * 22. / 24. + dt2 * (flux_p[ 1 ][ i - 1 ] - flux_p[ 1 ][ i ]);
      Mtx[ i ][ 3 ] = h / 24. + dt2 * (flux_p[ 2 ][ i - 1 ] - flux_p[ 2 ][ i ]);
      Mtx[ i ][ 4 ] = 0.;
   }

   for ( int i = 2; i < Ng; i += 2 ) {
      int k = i / 2;

      double h = 0.5 * (x1[ k ] - x1[ k - 1 ]);
      Mtx[ i ][ 0 ] = -h / 24. + dt2 * flux_p[ 0 ][ i - 1 ];
      Mtx[ i ][ 1 ] = h * 5. / 24. + dt2 * flux_p[ 1 ][ i - 1 ];
      Mtx[ i ][ 2 ] = h * 8. / 24. + dt2 * flux_p[ 2 ][ i - 1 ];

      h = 0.5 * (x1[ k + 1 ] - x1[ k ]);
      Mtx[ i ][ 2 ] += h * 8. / 24. - dt2 * flux_p[ 0 ][ i ];
      Mtx[ i ][ 3 ] = h * 5. / 24. - dt2 * flux_p[ 1 ][ i ];
      Mtx[ i ][ 4 ] = -h / 24. - dt2 * flux_p[ 2 ][ i ];
   }

   int i = 0;
   double h = 0.5 * (x1[ 1 ] - x1[ 0 ]);
   Mtx[ i ][ 2 ] = h * 8. / 24. - dt2 * flux_p[ 0 ][ 0 ];
   Mtx[ i ][ 3 ] = h * 5. / 24. - dt2 * flux_p[ 1 ][ 0 ];
   Mtx[ i ][ 4 ] = -h / 24. - dt2 * flux_p[ 2 ][ 0 ];

   i = Ng;
   h = 0.5 * (x1[ M1 ] - x1[ M1 - 1 ]);
   Mtx[ i ][ 0 ] = -h / 24. + dt2 * flux_p[ 0 ][ i - 1 ];
   Mtx[ i ][ 1 ] = h * 5. / 24. + dt2 * flux_p[ 1 ][ i - 1 ];
   Mtx[ i ][ 2 ] = h * 8. / 24. + dt2 * flux_p[ 2 ][ i - 1 ];

   ktim6 += (int) timer.restart();

   // assemble the right hand side
   i = 0;
   rhs[ i ] = IntQs(x0, u0, 0, -1., ke[ i ], xi[ i ]) + dt2 * flux_u[ i ];

   for ( i = 1; i < Ng; i++ ) {
      rhs[ i ] = IntQs(x0, u0, ke[ i - 1 ], xi[ i - 1 ], ke[ i ], xi[ i ]) + dt2 * (flux_u[ i ] - flux_u[ i - 1 ]);
   }

   i = Ng;
   rhs[ i ] = IntQs(x0, u0, ke[ i - 1 ], xi[ i - 1 ], M0 - 1, 1.) + dt2 * (-flux_u[ i - 1 ]);

   ktim7 += (int) timer.restart();
   LsSolver53(Ng, Mtx, rhs, u1);
   ktim8 += (int) timer.restart();

   for ( i = 0; i <= Ng; i++ )
      delete[] Mtx[ i ];

   delete[] Mtx;
   delete[] rhs;


   delete[] ke;
   delete[] MemDouble;
   DbgLv(2) << " Diff_C times 1-8" << ktim1 << ktim2 << ktim3 << ktim4 << ktim5 << ktim6 << ktim7 << ktim8;
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

void US_LammAstfvm::LocateStar(int N0, const double *x0, int Ns,  const double *xs, int *ke, double *xi) {
   int eix = 1;

   for ( int j = 0; j < Ns; j++ ) {
      while ( xs[ j ] > x0[ eix ] && eix < (N0 - 1)) eix++;

      ke[ j ] = eix - 1;
      xi[ j ] = (xs[ j ] - x0[ eix - 1 ]) / (x0[ eix ] - x0[ eix - 1 ]) * 2. - 1.;
   }
}

///////////////////////////////////////////////////////////////
//
//  Find the adjusted s and D values according to time t, location x,
//  and concentration C = u/x
//  (note: the input is u=r*C)
//
///////////////////////////////////////////////////////////////
void US_LammAstfvm::AdjustSD(double t, int Nv, double *x, const double *u, double *s_adj, double *D_adj) {
   const double Tempt = 293.15;    // temperature in K
   const double vbar_w = 0.72;
   const double rho_w = 0.998234;  //  density of water
   int jj;
   QVector<double> ViscVec(Nv);
   QVector<double> DensVec(Nv);
   double *Visc;
   double *Dens;
   double *Conc;
   double Cm = 0.0;
   double rho;
   double visc;
   //double  vbar   = 0.251;
   //double  vbar   = 0.72;      // 0.251;
   //double  vbar   = model.components[ 0 ].vbar20;
   double vbar = model.components[ comp_x ].vbar20;
   QElapsedTimer timer;
   static int kst1 = 0;
   static int kst2 = 0;

   switch ( NonIdealCaseNo ) {
      case 0:      // ideal, s=s_0, D=D_0

         for ( jj = 0; jj < Nv; jj++ ) {
            s_adj[ jj ] = param_s;
            D_adj[ jj ] = param_D;
         }
         break;

      case 1:      // concentration dependent
         for ( jj = 0; jj < Nv; jj++ ) {
            s_adj[ jj ] = param_s / (1. + sigma * u[ jj ] / x[ jj ]);
            D_adj[ jj ] = param_D / (1. + delta * u[ jj ] / x[ jj ]);
         }
         break;

      case 2:      // co-sedimenting
         //** salt-protein
         timer.start();

         ViscVec.resize(Nv);
         Visc = ViscVec.data();
         DensVec.resize(Nv);
         Dens = DensVec.data();
         for ( int j = 0; j < Nv; j++ )      // loop for all x[m]
         {
            Dens[ j ] = density;
            Visc[ j ] = viscosity;
         }
         DbgLv(2) << "NonIdeal2: ntrp Salt";
         if (cosed_needed){
            saltdata->InterpolateCCosed(Nv, x, t, Dens, Visc);}
         if (codiff_needed){
            bandFormingGradient->interpolateCCodiff(Nv, x, t, Dens, Visc);}

         if (dbg_level > 0){
            // calculate density and viscosity mean avg std
            DbgLv(2) << "#####################################";
            DbgLv(2) << "LFVM:AdjustSD: dens visc: t" << t << param_s20w << param_D20w;
            double dimn = 9e+14;
            double dimx = 0.0;
            double diav = 0.0;
            double vimn = 9e+14;
            double vimx = 0.0;
            double viav = 0.0;
//            double dltr = (af_data.radius[ 1 ] - af_data.radius[ 0 ]) * 0.5;


            double dsum = 0.0;
            double vsum = 0.0;

            for ( int jjj = 0; jjj < Nv; jjj++ ) {
               double dval = Dens[ jjj ];
               double vval = Visc[ jjj ];
               dsum += dval;
               vsum += vval;
               dimn = (dimn < dval) ? dimn : dval;
               dimx = (dimx > dval) ? dimx : dval;
               vimn = (vimn < vval) ? vimn : vval;
               vimx = (vimx > vval) ? vimx : vval;
               diav += dval;
               viav += vval;
            }
            DbgLv(2) << "Integral dens visc" << dsum << vsum;
            diav /= (double) Nv;
            double didf = dimx - dimn;
            double didp = (double) (qRound(10000.0 * didf / diav)) / 100.0;
            DbgLv(2) << "  Density Min Max Mean" << dimn << dimx << diav;
            DbgLv(2) << "  ( range of" << didf << "=" << didp << " percent of mean )";
            DbgLv(2) << "-------------";
            viav /= (double) Nv;
            double vidf = vimx - vimn;
            double vidp = (double) (qRound(10000.0 * vidf / viav)) / 100.0;
            DbgLv(2) << "  Viscosity Min Max Mean" << vimn << vimx << viav;
            DbgLv(2) << "  ( range of" << vidf << "=" << vidp << " percent of mean )";
         }

         kst1 += (int) timer.restart();
         {
            double smin = param_s;
            double dmin = param_D;
            double smax = 0.0;
            double dmax = 0.0;
            int sneg_in = 0;
            int dneg_in = 0;
            bool log_need = false;
            for ( jj = 0; jj < Nv; jj++ ) {
               US_Math2::SolutionData sol_data{};
               sol_data.density = Dens[ jj ];
               sol_data.viscosity = Visc[ jj ];
               sol_data.vbar20 = model.components[ comp_x ].vbar20; //The assumption here is that vbar does not change with
               sol_data.vbar = model.components[ comp_x ].vbar20; //temp, so vbar correction will cancel in s correction
               sol_data.manual = manual;
               US_Math2::data_correction(simparams.temperature, sol_data);
               s_adj[ jj ] = param_s20w / sol_data.s20w_correction;
               if (param_s20w / sol_data.s20w_correction < smin){
                  if (smin > 0.0 && param_s20w / sol_data.s20w_correction < 0.0) sneg_in = jj;
                  smin = param_s20w / sol_data.s20w_correction;
               }
               if (param_s20w / sol_data.s20w_correction > smax){
                  smax = param_s20w / sol_data.s20w_correction;
               }
               D_adj[ jj ] = param_D20w / sol_data.D20w_correction;
               if (param_D20w / sol_data.D20w_correction < dmin){
                  if (dmin > 0.0 && param_D20w / sol_data.D20w_correction < 0.0) dneg_in = jj;
                  dmin = param_D20w / sol_data.D20w_correction;
               }
               if (param_D20w / sol_data.D20w_correction > dmax){
                  dmax = param_D20w / sol_data.D20w_correction;
               }
               //D_adj[ jj ] = qAbs( dA / visc );
            }
            if (dbg_level > 0){
               DbgLv(2) << "AdjSD:  s or D negative: t" << t << param_s << param_D << Nv << param_s20w << param_D20w;
               DbgLv(2) << "AdjSD:  smin smax sneg_in r(sneg_in)" << smin << smax << sneg_in << x[sneg_in];
               DbgLv(2) << "AdjSD:  dmin dmax dneg_in r(dneg_in)" << dmin << dmax << dneg_in << x[dneg_in];
               log_need = true;
            }
            if (log_need){
            DbgLv(2) << "AdjSD:    sadj 0 m n" << s_adj[ 0 ] << s_adj[ Nv / 2 ] << s_adj[ Nv - 1 ];
            DbgLv(2) << "AdjSD:    Dadj 0 m n" << D_adj[ 0 ] << D_adj[ Nv / 2 ] << D_adj[ Nv - 1 ];
            DbgLv(2) << "AdjSD:    rho 0,m,e" << Dens[ 0 ] << Dens[ Nv / 2 ] << Dens[ Nv - 1 ];
            DbgLv(2) << "AdjSD:    visc 0,m,e" << Visc[ 0 ] << Visc[ Nv / 2 ] << Visc[ Nv - 1 ];
            DbgLv(2) << "AdjSD:    vbar vbar_w rho_w" << vbar << vbar_w << rho_w;}
         }

         kst2 += (int) timer.restart();
         DbgLv(3) << "AdjSD:  times 1 2" << kst1 << kst2;
         break;

      case 3:      // compressibility
      {
         double phip = vbar;     // apparent specific volume
         double alpha = 1.0;
         double factn = 0.5 * density * param_w2 * compressib;
         double msq = param_m * param_m;
         double sA = 1.0 - vbar * rho_w;

         for ( jj = 0; jj < Nv; jj++ ) {
            rho = density / (1.0 - factn * (x[ jj ] * x[ jj ] - msq));
            double beta = (1.0 - phip * rho) / sA;
            s_adj[ jj ] = param_s20w * alpha * beta;
            D_adj[ jj ] = param_D20w * alpha;
         }
         DbgLv(3) << "AdjSD: compr dens" << compressib << density;
         DbgLv(3) << "AdjSD:    factn msq sa" << factn << msq << sA;
         DbgLv(3) << "AdjSD:   sadj 0 m n" << s_adj[ 0 ] << s_adj[ Nv / 2 ] << s_adj[ Nv - 1 ];
         DbgLv(3) << "AdjSD:   Dadj 0 m n" << D_adj[ 0 ] << D_adj[ Nv / 2 ] << D_adj[ Nv - 1 ];
      }
         break;
      case 4: // co-diffusing
      {
         timer.start();
         DbgLv(2) << "NonIdeal4";
         kst1 += (int) timer.restart();
         for ( jj = 0; jj < Nv; jj++ ) {
            double adj_s = param_s;
            double adj_d = param_D;
            DbgLv(2) << "AdjSD:   Cosed t x tmp vbar" << t << x[ jj ] << simparams.temperature << vbar;
            DbgLv(2) << "AdjSD:   CoSed s" << QString::number(adj_s * 1E+13, 'f', 4);
            DbgLv(2) << "AdjSD:   CoSed D" << QString::number(adj_d * 1E+6, 'f', 4);
            bandFormingGradient->adjust_sd(x[ jj ], t, adj_s, adj_d, simparams.temperature, vbar);
            s_adj[ jj ] = adj_s;
            D_adj[ jj ] = adj_d;
            DbgLv(2) << "AdjSD:   CoSed s_adj" << QString::number(adj_s * 1E+13, 'f', 4);
            DbgLv(2) << "AdjSD:   CoSed D_adj" << QString::number(adj_d * 1E+6, 'f', 4);
         }
      }
         kst2 += (int) timer.restart();
         DbgLv(3) << "AdjSD:  times 1 2" << kst1 << kst2;
         break;

      default:
         qDebug("invalid case number for non-ideal sedimentation");
         break;

   } // switch
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

void US_LammAstfvm::fun_phi(double x, double *y) {
   double x2 = x * x;
   y[ 0 ] = 0.5 * (x2 - x);
   y[ 1 ] = 1.0 - x2;
   y[ 2 ] = 0.5 * (x2 + x);
}

void US_LammAstfvm::fun_dphi(double x, double *y) {
   // quadratic basis
   double x2 = x * x;
   y[ 0 ] = 0.5 * (x2 - x);
   y[ 1 ] = 1.0 - x2;
   y[ 2 ] = 0.5 * (x2 + x);

   // derivatives
   y[ 3 ] = x - 0.5;
   y[ 4 ] = -2. * x;
   y[ 5 ] = x + 0.5;
}

void US_LammAstfvm::fun_Iphi(double x, double *y) {
   double x2 = x * x;
   double x3 = x * x2 / 6.0;
   x2 *= 0.25;
   y[ 0 ] = x3 - x2;
   y[ 1 ] = x - x3 * 2.0;
   y[ 2 ] = x3 + x2;
}

////////////////////////////////////////////
//
// integrate a quadratic function defined on (x[0], x[1])
// by nodal values u[0], u[1], u[2] from xi=xia to xib
// here x=x[0]+(x[1]-x[0])*(xi+1)/2
//
////////////////////////////////////////////
double US_LammAstfvm::IntQ(const double *x, const double *u, double xia, double xib) {
   double intgrl;
   double phia[3];
   double phib[3];

   fun_Iphi(xia, phia);
   fun_Iphi(xib, phib);

   intgrl = (x[ 1 ] - x[ 0 ]) / 2. *
            (u[ 0 ] * (phib[ 0 ] - phia[ 0 ]) + u[ 1 ] * (phib[ 1 ] - phia[ 1 ]) + u[ 2 ] * (phib[ 2 ] - phia[ 2 ]));

   return (intgrl);
}

//////////////////////////////////////////////////////////////////
//
// integrate a piecewise quadratic function defined on mesh *x
// by nodal values *u from xia in elem ka to xib in elem kb
//
//////////////////////////////////////////////////////////////////
double US_LammAstfvm::IntQs(double *x, double *u, int ka, double xia, int kb, double xib) {
   double intgrl;

   if ( ka == kb ) {
      intgrl = IntQ(x + ka, u + 2 * ka, xia, xib);
   } else       // integral across different elems of mesh x
   {
      intgrl = IntQ(x + ka, u + 2 * ka, xia, 1.);
      intgrl += IntQ(x + kb, u + 2 * kb, -1., xib);

      for ( int k = ka + 1; k <= kb - 1; k++ ) {
         int k2 = k + k;
         intgrl += (x[ k + 1 ] - x[ k ]) * (u[ k2 ] + u[ k2 + 2 ] + 4. * u[ k2 + 1 ]) / 6.;
      }
   }

   return (intgrl);
}

/////////////////////////////////////////////////////////////////
//
// Interpolation-projection of a piecewise quadratic u0 on x0
// onto piecewise quadratic on mesh x1
//
// M0, M1 = number of elems in x0 and x1
//
/////////////////////////////////////////////////////////////////
void US_LammAstfvm::ProjectQ(int M0, double *x0, double *u0, int M1, double *x1, double *u1) {
   double intgrl;
   double phi[3];

   int *ke = new int[M1 + 1];
   auto *xi = new double[M1 + 1];

   LocateStar(M0 + 1, x0, M1 + 1, x1, ke, xi);

   // u1 = u0 at all nodes
   for ( int j = 0; j <= M1; j++ ) {
      fun_phi(xi[ j ], phi);

      int idx = 2 * ke[ j ];
      u1[ 2 * j ] = phi[ 0 ] * u0[ idx ] + phi[ 1 ] * u0[ idx + 1 ] + phi[ 2 ] * u0[ idx + 2 ];
   }

   for ( int j = 0; j < M1; j++ ) {
      int j2 = 2 * j;

      intgrl = IntQs(x0, u0, ke[ j ], xi[ j ], ke[ j + 1 ], xi[ j + 1 ]);

      u1[ j2 + 1 ] = 1.5 * intgrl / (x1[ j + 1 ] - x1[ j ]) - 0.25 * (u1[ j2 ] + u1[ j2 + 2 ]);
   }

   delete[] ke;
   delete[] xi;
}


///////////////////////////////////////////////////////
//
// LsSolver53
// Matrix A[m+1][5]: with A[i][2]=a_ii diagonal
//                        A[i][0]=A[i][4]=0, for i = odd
// m must be even
//
///////////////////////////////////////////////////////
void US_LammAstfvm::LsSolver53(int m, double **A, double *b, double *x) {

   for ( int j = 0; j < m - 1; j += 2 ) {
      int j1 = j + 1;
      int j2 = j + 2;

      double multi = -A[ j1 ][ 1 ] / A[ j ][ 2 ];
      A[ j1 ][ 2 ] += multi * A[ j ][ 3 ];
      A[ j1 ][ 3 ] += multi * A[ j ][ 4 ];
      b[ j1 ] += multi * b[ j ];

      multi = -A[ j2 ][ 0 ] / A[ j ][ 2 ];
      A[ j2 ][ 1 ] += multi * A[ j ][ 3 ];
      A[ j2 ][ 2 ] += multi * A[ j ][ 4 ];
      b[ j2 ] += multi * b[ j ];

      multi = -A[ j2 ][ 1 ] / A[ j1 ][ 2 ];
      A[ j2 ][ 2 ] += multi * A[ j1 ][ 3 ];
      b[ j2 ] += multi * b[ j1 ];
   }

   // Back-substitution
   x[ m ] = b[ m ] / A[ m ][ 2 ];

   for ( int j = m - 1; j > 0; j -= 2 ) {
      int jm = j - 1;
      int jp = j + 1;

      x[ j ] = (b[ j ] - A[ j ][ 3 ] * x[ jp ]) / A[ j ][ 2 ];

      x[ jm ] = (b[ jm ] - A[ jm ][ 3 ] * x[ j ] - A[ jm ][ 4 ] * x[ jp ]) / A[ jm ][ 2 ];
   }

}

// determine the non-ideal case number: 0/1/2/3/4
int US_LammAstfvm::nonIdealCaseNo() {
   int rc = 0;

//   if ( comp_x > 0 )
//      return rc;

   NonIdealCaseNo = 0;      // ideal

   US_Model::SimulationComponent *sc = &model.components[ comp_x ];

   if ( sc->sigma != 0.0 || sc->delta != 0.0 ) {  // non-zero sigma or delta given:         concentration-dependent
      NonIdealCaseNo = 1;
   }

   if ( compressib > 0.0 ) {  // compressibility factor positive:       compressibility
      if ( NonIdealCaseNo != 0 )
         rc = 3;

      NonIdealCaseNo = 3;
   }

   if ( !cosed_components.isEmpty()) {
      // co-diffusing case
      if ( NonIdealCaseNo != 0 )
         rc = 2;

      NonIdealCaseNo = 2;
   }
   DbgLv(1) << "LammAstfvm: set nonidealcaseno:" << NonIdealCaseNo;
   return rc;
}

// perform quadratic interpolation to fill out full concentration vector
void US_LammAstfvm::quadInterpolate(const double *x0, const double *u0, int N0, QVector<double> &xout, QVector<double> &cout) {
   int nout = xout.size();         // output concentrations count
   int kk = 0;                   // initial output index
   double xv = xout[ 0 ];           // first output X
   double yv;                         // output Y

   int ii = 2;                   // next x0 index to x3
   int jj = 3;                   // next u0 index to y2
   double x1 = x0[ 0 ];             // initial start X
   double x3 = x0[ 1 ];             // initial end X
   double x2 = (x1 + x3) * 0.5;   // initial mid-point X
   double y1 = u0[ 0 ];             // initial start Y
   double y2 = u0[ 1 ];             // initial mid-point Y
   double y3 = u0[ 2 ];             // initial end Y

   cout.resize(nout);

   while ( kk < nout ) {  // loop to output interpolated concentrations
      xv = xout[ kk ];              // X for which we need a Y

      while ( xv > x3 && ii < N0 ) {  // if need be, walk up input until between x values
         x1 = x3;                   // start x (previous end)
         x3 = x0[ ii++ ];           // end (next) x
         y1 = y3;                   // y at start x
         y2 = u0[ jj++ ];           // y at mid-point
         y3 = u0[ jj++ ];           // y at end (next) x

      }
      x2 = (x1 + x3) * 0.5;    // mid-point x
//y1 /= x1;
//y2 /= x2;
//y3 /= x3;

      // do the quadratic interpolation of this Y (C*r)
      yv    =
         (( ( xv - x2 ) * ( xv - x3 ) ) / ( ( x1 - x2 ) * ( x1 - x3 ) )) * y1 +
         (( ( xv - x1 ) * ( xv - x3 ) ) / ( ( x2 - x1 ) * ( x2 - x3 ) )) * y2 +
         (( ( xv - x1 ) * ( xv - x2 ) ) / ( ( x3 - x1 ) * ( x3 - x2 ) )) * y3;
       double yv1    =
               (( ( xv - x2 ) * ( xv - x3 ) ) / ( ( x1 - x2 ) * ( x1 - x3 ) )) * y1/x1 +
               (( ( xv - x1 ) * ( xv - x3 ) ) / ( ( x2 - x1 ) * ( x2 - x3 ) )) * y2/x2 +
               (( ( xv - x1 ) * ( xv - x2 ) ) / ( ( x3 - x1 ) * ( x3 - x2 ) )) * y3/x3;
       if (abs(yv/xv-yv1)>1e-5){
           qDebug() << "yv yv1 xv" << QString::number(yv,'e',7) << QString::number(yv1,'e',7) << QString::number(xv,'f',6);
           qDebug() << "x1 y1" << QString::number(x1,'f',6) << QString::number(y1,'e',7);
           qDebug() << "x2 y2" << QString::number(x2,'f',6) << QString::number(y2,'e',7);
           qDebug() << "x3 y3" << QString::number(x3,'f',6) << QString::number(y3,'e',7);
           qDebug() << "----";
       }
//y1 *= x1;
//y2 *= x2;
//y3 *= x3;

      // output interpolated concentration with r factor removed (C = (C*r)/r)
      cout[ kk++ ] = yv1;
//cout[ kk++ ] = yv;
   }

}

// load MfemData object used internally from caller's RawData object
void US_LammAstfvm::load_mfem_data(US_DataIO::RawData &edata, US_AstfemMath::MfemData &fdata, bool zeroout) const {
   int nscan = edata.scanData.size();  // scan count

//   int  nconc  = edata.x.size();         // concentrations count
   int nconc = edata.xvalues.size();   // concentrations count
   DbgLv(1) << "Lamm:ldMFEM: nscan edata.scanCount() nconc edata.pointCount()" << nscan << edata.scanCount() << nconc
            << edata.pointCount();
   fdata.id = edata.description;
   fdata.cell = edata.cell;
   fdata.scan.resize(nscan);         // mirror number of scans
   fdata.radius.resize(nconc);         // mirror number of radius values
   DbgLv(1) << "RSA:f  r0 rn" << fdata.radius[ 0 ] << fdata.radius[ nconc - 1 ];

   for ( int ii = 0; ii < nscan; ii++ ) {  // copy over all scans
      US_AstfemMath::MfemScan *fscan = &fdata.scan[ ii ];

      fscan->temperature = edata.scanData[ ii ].temperature;
      fscan->rpm = edata.scanData[ ii ].rpm;
      fscan->time = edata.scanData[ ii ].seconds;
      fscan->omega_s_t = edata.scanData[ ii ].omega2t;
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
         fscan->conc.fill(0.0, nconc); // if so specified, set concentrations to zero

      else                        // Otherwise, copy concentrations
         fscan->conc = edata.scanData[ ii ].rvalues;
   }

#endif
   fdata.radius = edata.xvalues;
   int nn = fdata.radius.size() - 1;
   int mm = nn / 2;
   DbgLv(1) << "LdDa:  n r0 rm rn" << nn << fdata.radius[ 0 ] << fdata.radius[ mm ] << fdata.radius[ nn ];
   DbgLv(1) << "RSA:f sc0 temp" << fdata.scan[ 0 ].temperature;
   DbgLv(1) << "RSA:e sc0 temp" << edata.scanData[ 0 ].temperature;
}

// store MfemData object used internally into caller's RawData object
void US_LammAstfvm::store_mfem_data(US_DataIO::RawData &edata, US_AstfemMath::MfemData &fdata) const {
   int nscan = fdata.scan.size();     // scan count
   int nconc = fdata.radius.size();   // concentrations count
   int escan = edata.scanCount();
   int econc = edata.pointCount();
   DbgLv(2) << "Lamm:st_md: nscan nconc" << nscan << nconc;
   DbgLv(2) << "Lamm:st_md: escan econc" << escan << econc;

   edata.description = fdata.id;
   edata.cell = fdata.cell;
   edata.xvalues = fdata.radius;
   if ( escan != nscan )
      edata.scanData.resize(nscan);      // mirror number of scans

   for ( int ii = 0; ii < nscan; ii++ ) {  // copy over each scan
      US_AstfemMath::MfemScan *fscan = &fdata.scan[ ii ];
      US_DataIO::Scan *eescan = &edata.scanData[ ii ];

      eescan->temperature = fscan->temperature;
      eescan->rpm = fscan->rpm;
      eescan->seconds = fscan->time;
      eescan->omega2t = fscan->omega_s_t;
      eescan->plateau = fdata.radius[ nconc - 1 ];
      eescan->rvalues = fscan->conc;
   }
   DbgLv(2) << "Lamm:o-f sc0 temp" << fdata.scan[ 0 ].temperature;
   DbgLv(2) << "Lamm:o-e sc0 temp" << edata.scanData[ 0 ].temperature;
}

void US_LammAstfvm::setStopFlag(bool flag) {
   stopFlag = flag;
   qApp->processEvents();
   DbgLv(2) << "setStopFlag" << stopFlag;
}

void US_LammAstfvm::setMovieFlag(bool flag) {
   movieFlag = flag;
   qApp->processEvents();
   DbgLv(2) << "setMovieFlag" << movieFlag;
}

void US_LammAstfvm::validate_bfg() {
    bool co_diff = false;
    foreach (US_CosedComponent cosedComponent, cosed_components){
        if (cosedComponent.overlaying && cosedComponent.s_coeff == 0.0){
            co_diff = true;
            break;
        }
    }
   DbgLv(1) << "validated bfg" << co_diff;
    if (!co_diff){
        bandFormingGradient = nullptr;
        return;
    }
    // check meniscus, bottom, simvalues and components
    if (cosed_components != bandFormingGradient->cosed_component ||
    simparams.meniscus != bandFormingGradient->meniscus ||
    simparams.bottom != bandFormingGradient->bottom ||
    abs(simparams.band_volume - bandFormingGradient->overlay_volume) > GSL_ROOT5_DBL_EPSILON ||
    abs( simparams.cp_pathlen - bandFormingGradient->cp_pathlen) > GSL_ROOT5_DBL_EPSILON ||
    abs( simparams.cp_angle - bandFormingGradient->cp_angle) > GSL_ROOT5_DBL_EPSILON ||
    simparams.radial_resolution != bandFormingGradient->simparms.radial_resolution ||
    simparams.temperature != bandFormingGradient->simparms.temperature ||
    auc_data->scanData.last().seconds > bandFormingGradient->dens_bfg_data.scanData.last().seconds){
        // recalculation needed
        bandFormingGradient = new US_Math_BF::Band_Forming_Gradient(simparams.meniscus, simparams.bottom, simparams.band_volume,
                                                      cosed_components, simparams.cp_pathlen, simparams.cp_angle);
        bandFormingGradient->get_eigenvalues();
        bandFormingGradient->calculate_gradient(simparams, auc_data);
       DbgLv(1) << "validated bfg recalculated";
    }
   DbgLv(1) << "validated bfg";
}

void US_LammAstfvm::validate_csd() {
    bool co_sed = false;
    US_Model cosed_model_tmp;
    US_Model cosed_model;
    US_Math2::SolutionData sol_data{};
    sol_data.density = density;
    sol_data.viscosity = viscosity;
    sol_data.manual = true;

    foreach(US_CosedComponent cosed_comp,  cosed_components) {
        // get the excess concentrations
        if (cosed_comp.s_coeff == 0.0){
            DbgLv(1) << "pure diffusive";
            codiff_needed = true;
            continue;
        }
        cosed_needed = true;
        cosed_model_tmp.components.clear();
        US_Model::SimulationComponent tmp = US_Model::SimulationComponent();
        tmp.name = cosed_comp.name;
        tmp.analyteGUID = cosed_comp.GUID;
        tmp.molar_concentration = cosed_comp.conc;
        tmp.signal_concentration = cosed_comp.conc;
        tmp.vbar20 = cosed_comp.vbar;
        if (cosed_comp_data.contains(tmp.analyteGUID)){
            continue;}
        sol_data.vbar20 = cosed_comp.vbar; //The assumption here is that vbar does not change with
        sol_data.vbar = cosed_comp.vbar; //temp, so vbar correction will cancel in s correction
        US_Math2::data_correction(simparams.temperature, sol_data);
        tmp.s = cosed_comp.s_coeff / sol_data.s20w_correction;
        tmp.D = cosed_comp.d_coeff / sol_data.D20w_correction;
        tmp.f_f0 = 0.0;
        tmp.analyte_type = 4;
        cosed_model.components << tmp;
        cosed_model_tmp.components << tmp;
        cosed_model_tmp.update_coefficients();
        saltdata = new US_LammAstfvm::CosedData(cosed_model_tmp, simparams, auc_data, &cosed_components,
                                           density, viscosity);
        cosed_comp_data[ tmp.analyteGUID ] = saltdata->sa_data;
        DbgLv(2) << "NonIdeal2: create saltdata";
        cosed_model.update_coefficients();
        saltdata->model = cosed_model;
        saltdata->cosed_comp_data = cosed_comp_data;
        saltdata->cosed_comp_data.detach();
        DbgLv(1) << "CosedData: cosed_model comp" << saltdata->model.components.size() << "cosed_comp_data"
                 << cosed_comp_data.size() << "sa_data.scanCount()" << saltdata->sa_data.scanCount();
    }
    if (!cosed_comp_data.isEmpty()){
        saltdata->sa_data= cosed_comp_data.first();}
    if (!co_sed) {
        saltdata = nullptr;
        return;
    }
}