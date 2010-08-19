//! \file us_lamm_astfvm.h
#ifndef US_LAMMASTFVM_H
#define US_LAMMASTFVM_H

#include <QtCore>
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_dataIO2.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"

//! \brief Module to calculate simulation data by AST Finite Volume Method
class US_EXTERN US_LammAstfvm : QObject
{
   Q_OBJECT

   public:
      class Mesh 
      {
   
         public: 
            //unsigned int Nv, Ne;	// Number of grids, and Number of elems
            int Nv, Ne;	// Number of grids, and Number of elems
            double *x;			// coordinates of grids
     
            Mesh(double xl, double xr, int Nelem, int opt);
            ~Mesh();
            void InitMesh(double s, double D, double w2);
            void RefineMesh(double *u0, double *u1, double ErrTol);

         private:
            int MaxRefLev;
            int MonScale;
            double MonCutoff;
            double SmoothingWt;
            int    SmoothingCyl;

            int *Eid;			// elem id
            int *RefLev;	        // refinement level of an elem
            double *MeshDen;		// desired mesh density
            int *Mark;			// ref/unref marker

            void ComputeMeshDen_D3(double *u0, double *u1);
            //void Smoothing(unsigned int n, double *y, double Wt, unsigned int Cycle);
            void Smoothing( int n, double *y, double Wt, int Cycle);
            void Unrefine(double alpha);
            void Refine(double beta);
      };


      class SaltData
      {
         public:

            double SaltMoler;		// moler number of cosedimenting salt

            SaltData(char *fname, double Moler);
            ~SaltData();
            void InterpolateCSalt(int N, double *x, double t, double *Csalt);

            private:

            FILE *f_salt;
            int Nx;		// number of points in radial direction
            int Nt;		// number of points in time direction
            double *xs;	// grids in radial direction
            double t0, t1;	// time intervals in use.
            double *Cs0, *Cs1; // salt concentration for the time interval
      };

      //US_LammAstfvm( US_Model&, US_SimulationParameters&, QObject* = 0 );

      US_LammAstfvm(double m, double b, double s, double D, double rpm); 

      ~US_LammAstfvm();

      int calculate( US_DataIO2::RawData& );


      Mesh *msh;			    // redial grid

      int NonIdealCaseNo;		    // non-ideal case number
                                 // = 0 : ideal, constant s, D
                                 // = 1 : concentration dependent
                                 // = 2 : co-sedimenting

      double ConcDep_K;		    // constant for concentration dependent
                                 // non-ideal case
                                 // s = s_0/(1+K*C), D=D_0/(1+K*C)

      SaltData *saltdata;		    // data handle for cosedimenting

      double MeshSpeedFactor;          // = 1: mesh following sedimentation
                                     // = 0: fixed mesh in each time step

      int MeshRefineOpt;               // = 1: perform mesh local refinement
                                    // = 0: no mesh refinement


      void SetNonIdealCase_1( double const_K );	// conc dependent case

      void SetNonIdealCase_2( char *fname, double Moler );  // co-sed case

      void SetMeshSpeedFactor(double speed);	// moving (1) or not moving (0)

      void SetMeshRefineOpt(int Opt);	        // Refine (1) or no-refine (0)

      void LammStepSedDiff_P(double t, double dt, int M0, double *x0, double *u0, 
                             double *u1 );

      void LammStepSedDiff_C(double t, double dt, int M0, double *x0, double *u0, 
                             int M1, double *x1, double *u1_est, double *u1 );

      void ProjectQ(int M0, double *x0, double *u0, 
                    int M1, double *x1, double *u1);

      double IntQs(double *x, double *u, int ka, double xia, int kb, double xib);

      ///////
   private:

      double param_m, param_b;	// m, b of cell
      double param_s, param_D; 	// base s, D values
      double param_w2;		// rpm, w2=(rpm*pi/30)^2


      void LocateStar(int N0, double *x0, int Ns, double *xs,
                      int *ke, double *xi);


      void AdjustSD(double t, int Nv, double *x, double *u, 
                    double *s_adj, double *D_adj);

      void fun_phi(double x, double *y);

      void fun_dphi(double x, double *y);

      void fun_Iphi(double x, double *y);

      double IntQ(double *x, double *u, double xia, double xib);


      void LsSolver53(int m, double **A, double *b, double *x);

};
#endif
