<#include <pthread.h>
#define SHOW_TIMING
#if defined(SHOW_TIMING)
# include <sys/time.h>
  struct timeval start_tv, end_tv;
#endif

/*****************************************************************************

  nnls.c  (c) 2002 Turku PET Centre

  This file contains the routine NNLS (nonnegative least squares)
  and the subroutines required by it.
  
  This routine is based on the text and fortran code in
  C.L. Lawson and R.J. Hanson, Solving Least Squares Problems,
  Prentice-Hall, Englewood Cliffs, New Jersey, 1974.

  Version:
  2002-08-19 Vesa Oikonen


*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/****************************************************************************/
#include "include/nnls.h"


/****************************************************************************/
/* Local function definitions */
void _nnls_g1(double a, double b, double *cterm, double *sterm, double *sig);
int _nnls_h12(int mode, int lpivot, int l1, int m, double *u, int iue,
              double *up, double *cm, int ice, int icv, int ncv);
/****************************************************************************/

/*****************************************************************************
 *  Algorithm NNLS (Non-negative least-squares)
 *
 *  Given an m by n matrix A, and an m-vector B, computes an n-vector X,
 *  that solves the least squares problem
 *      A * X = B   , subject to X>=0
 *
 *  Function returns 0 if succesful, 1, if iteration count exceeded 3*N,
 *  or 2 in case of invalid problem dimensions or memory allocation error.
 *
 *  Instead of pointers for working space, NULL can be given to let this
 *  function to allocate and free the required memory.
 */
int nnls_thread(
  double *a, int a_dim1, int m, int n,
  /* On entry, a[] contains the m by n matrix A. On exit, a[] contains the
     product matrix Q*A, where Q is an m by n orthogonal matrix generated
     implicitly by this function. Since matrix A is processed as a set of
     vectors, a_dim1 is needed to specify the storage increment between
     vectors in a[] */
  double *b,
  /* On entry, b[] must contain the m-vector B.
     On exit, b[] contains Q*B */
  double *x,
  /* On exit, x[] will contain the solution vector */
  double *rnorm,
  /* On exit, rnorm contains the Euclidean norm of the residual vector */
  double *wp,  /* An n-array of working space, w[]. */
  /* On exit, w[] will contain the dual solution vector.
     w[i]=0.0 for all i in set p and w[i]<=0.0 for all i in set z. */
  double *zzp, /* An m-array of working space, zz[]. */
  int *indexp  /* An n-array of working space, index[]. */
) {
  int pfeas, ret=0, iz, jz, iz1, iz2, npp1, *index;
  double d1, d2, sm, up, ss, *w, *zz;
  int iter, k, j=0, l, itmax, izmax=0, nsetp, ii, jj=0, ip;
  double temp, wmax, t, alpha, asave, dummy, unorm, ztest, cc;


  /* Check the parameters and data */
  if(m<=0 || n<=0 || a==NULL || b==NULL || x==NULL) return(2);
  /* Allocate memory for working space, if required */
  if(wp!=NULL) w=wp; else w=(double*)calloc(n, sizeof(double));
  if(zzp!=NULL) zz=zzp; else zz=(double*)calloc(m, sizeof(double));
  if(indexp!=NULL) index=indexp; else index=(int*)calloc(n, sizeof(int));
  if(w==NULL || zz==NULL || index==NULL) return(2);

  /* Initialize the arrays INDEX[] and X[] */
  for(k=0; k<n; k++) {x[k]=0.; index[k]=k;}
  iz2=n-1; iz1=0; nsetp=0; npp1=0;

  /* Main loop; quit if all coeffs are already in the solution or */
  /* if M cols of A have been triangularized */
  iter=0; itmax=n*3;
  while(iz1<=iz2 && nsetp<m) {
    /* Compute components of the dual (negative gradient) vector W[] */
    for(iz=iz1; iz<=iz2; iz++) {
      j=index[iz]; sm=0.; for(l=npp1; l<m; l++) sm+=a[l+j*a_dim1]*b[l];
      w[j]=sm;
    }

    while(1) {
      /* Find largest positive W[j] */
      for(wmax=0., iz=iz1; iz<=iz2; iz++) {
        j=index[iz]; if(w[j]>wmax) {wmax=w[j]; izmax=iz;}}

      /* Terminate if wmax<=0.; */
      /* it indicates satisfaction of the Kuhn-Tucker conditions */
      if(wmax<=0.0) break;
      iz=izmax; j=index[iz];

      /* The sign of W[j] is ok for j to be moved to set P. */
      /* Begin the transformation and check new diagonal element to avoid */
      /* near linear dependence. */
      asave=a[npp1+j*a_dim1];
      _nnls_h12(1, npp1, npp1+1, m, &a[j*a_dim1], 1, &up, &dummy, 1, 1, 0);
      unorm=0.;
      if(nsetp!=0) for(l=0; l<nsetp; l++) {d1=a[l+j*a_dim1]; unorm+=d1*d1;}
      unorm=sqrt(unorm);
      d2=unorm+(d1=a[npp1+j*a_dim1], fabs(d1)) * 0.01;
      if((d2-unorm)>0.) {
        /* Col j is sufficiently independent. Copy B into ZZ, update ZZ */
        /* and solve for ztest ( = proposed new value for X[j] ) */
        for(l=0; l<m; l++) zz[l]=b[l];
        _nnls_h12(2, npp1, npp1+1, m, &a[j*a_dim1], 1, &up, zz, 1, 1, 1);
        ztest=zz[npp1]/a[npp1+j*a_dim1];
        /* See if ztest is positive */
        if(ztest>0.) break;
      }

      /* Reject j as a candidate to be moved from set Z to set P. Restore */
      /* A[npp1,j], set W[j]=0., and loop back to test dual coeffs again */
      a[npp1+j*a_dim1]=asave; w[j]=0.;
    } /* while(1) */
    if(wmax<=0.0) break;

    /* Index j=INDEX[iz] has been selected to be moved from set Z to set P. */
    /* Update B and indices, apply householder transformations to cols in */
    /* new set Z, zero subdiagonal elts in col j, set W[j]=0. */
    for(l=0; l<m; ++l) b[l]=zz[l];
    index[iz]=index[iz1]; index[iz1]=j; iz1++; nsetp=npp1+1; npp1++;
    if(iz1<=iz2) for(jz=iz1; jz<=iz2; jz++) {
      jj=index[jz];
      _nnls_h12(2, nsetp-1, npp1, m, &a[j*a_dim1], 1, &up,
           &a[jj*a_dim1], 1, a_dim1, 1);
    }
    if(nsetp!=m) for(l=npp1; l<m; l++) a[l+j*a_dim1]=0.;
    w[j]=0.;
    /* Solve the triangular system; store the solution temporarily in Z[] */
    for(l=0; l<nsetp; l++) {
      ip=nsetp-(l+1);
      if(l!=0) for(ii=0; ii<=ip; ii++) zz[ii]-=a[ii+jj*a_dim1]*zz[ip+1];
      jj=index[ip]; zz[ip]/=a[ip+jj*a_dim1];
    }

    /* Secondary loop begins here */
    while(++iter<itmax) {
      /* See if all new constrained coeffs are feasible; if not, compute alpha */
      for(alpha=2.0, ip=0; ip<nsetp; ip++) {
        l=index[ip];
        if(zz[ip]<=0.) {t=-x[l]/(zz[ip]-x[l]); if(alpha>t) {alpha=t; jj=ip-1;}}
      }

      /* If all new constrained coeffs are feasible then still alpha==2. */
      /* If so, then exit from the secondary loop to main loop */
      if(alpha==2.0) break;
      /* Use alpha (0.<alpha<1.) to interpolate between old X and new ZZ */
      for(ip=0; ip<nsetp; ip++) {l=index[ip]; x[l]+=alpha*(zz[ip]-x[l]);}

      /* Modify A and B and the INDEX arrays to move coefficient i */
      /* from set P to set Z. */
      k=index[jj+1]; pfeas=1;
      do {
        x[k]=0.;
        if(jj!=(nsetp-1)) {
          jj++;
          for(j=jj+1; j<nsetp; j++) {
            ii=index[j]; index[j-1]=ii;
            _nnls_g1(a[j-1+ii*a_dim1], a[j+ii*a_dim1], &cc, &ss, &a[j-1+ii*a_dim1]);
            for(a[j+ii*a_dim1]=0., l=0; l<n; l++) if(l!=ii) {
              /* Apply procedure G2 (CC,SS,A(J-1,L),A(J,L)) */
              temp=a[j-1+l*a_dim1];
              a[j-1+l*a_dim1]=cc*temp+ss*a[j+l*a_dim1];
              a[j+l*a_dim1]=-ss*temp+cc*a[j+l*a_dim1];
            }
            /* Apply procedure G2 (CC,SS,B(J-1),B(J)) */
            temp=b[j-1]; b[j-1]=cc*temp+ss*b[j]; b[j]=-ss*temp+cc*b[j];
          }
        }
        npp1=nsetp-1; nsetp--; iz1--; index[iz1]=k;

        /* See if the remaining coeffs in set P are feasible; they should */
        /* be because of the way alpha was determined. If any are */
        /* infeasible it is due to round-off error. Any that are */
        /* nonpositive will be set to zero and moved from set P to set Z */
        for(jj=0; jj<nsetp; jj++) {k=index[jj]; if(x[k]<=0.) {pfeas=0; break;}}
      } while(pfeas==0);

      /* Copy B[] into zz[], then solve again and loop back */
      for(k=0; k<m; k++) zz[k]=b[k];
      for(l=0; l<nsetp; l++) {
        ip=nsetp-(l+1);
        if(l!=0) for(ii=0; ii<=ip; ii++) zz[ii]-=a[ii+jj*a_dim1]*zz[ip+1];
        jj=index[ip]; zz[ip]/=a[ip+jj*a_dim1];
      }
    } /* end of secondary loop */
    if(iter>itmax) {ret=1; break;}
    for(ip=0; ip<nsetp; ip++) {k=index[ip]; x[k]=zz[ip];}
  } /* end of main loop */
  /* Compute the norm of the final residual vector */
  sm=0.;
  if(npp1<m) for(k=npp1; k<m; k++) sm+=(b[k]*b[k]);
  else for(j=0; j<n; j++) w[j]=0.;
  *rnorm=sqrt(sm);
  /* Free working space, if it was allocated here */
  if(wp==NULL) free(w); if(zzp==NULL) free(zz); if(indexp==NULL) free(index);
  return(ret);
} /* nnls_ */
/****************************************************************************/

/*****************************************************************************
 *
 *  Compute orthogonal rotation matrix:
 *    (C, S) so that (C, S)(A) = (sqrt(A**2+B**2))
 *    (-S,C)         (-S,C)(B)   (   0          )
 *  Compute sig = sqrt(A**2+B**2):
 *    sig is computed last to allow for the possibility that sig may be in
 *    the same location as A or B.
 */
void _nnls_g1(double a, double b, double *cterm, double *sterm, double *sig)
{
  double d1, xr, yr;

  if(fabs(a)>fabs(b)) {
    xr=b/a; d1=xr; yr=sqrt(d1*d1 + 1.); d1=1./yr;
    *cterm=(a>=0.0 ? fabs(d1) : -fabs(d1));
    *sterm=(*cterm)*xr; *sig=fabs(a)*yr;
  } else if(b!=0.) {
    xr=a/b; d1=xr; yr=sqrt(d1*d1 + 1.); d1=1./yr;
    *sterm=(b>=0.0 ? fabs(d1) : -fabs(d1));
    *cterm=(*sterm)*xr; *sig=fabs(b)*yr;
  } else {
    *sig=0.; *cterm=0.; *sterm=1.;
  }
} /* _nnls_g1 */
/****************************************************************************/

/*****************************************************************************
 *
 *  Construction and/or application of a single Householder transformation:
 *           Q = I + U*(U**T)/B
 *
 *  Function returns 0 if succesful, or >0 in case of erroneous parameters.
 *
 */
int _nnls_h12(
  int mode,
  /* mode=1 to construct and apply a Householder transformation, or
     mode=2 to apply a previously constructed transformation */
  int lpivot,     /* Index of the pivot element */
  int l1, int m,
  /* Transformation is constructed to zero elements indexed from l1 to M */
  double *u, int u_dim1, double *up,
  /* With mode=1: On entry, u[] must contain the pivot vector.
     On exit, u[] and up contain quantities defining the vector u[] of
     the Householder transformation. */
  /* With mode=2: On entry, u[] and up should contain quantities previously
     computed with mode=1. These will not be modified. */
  /* u_dim1 is the storage increment between elements. */
  double *cm,
  /* On entry, cm[] must contain the matrix (set of vectors) to which the
     Householder transformation is to be applied. On exit, cm[] will contain
     the set of transformed vectors */
  int ice,        /* Storage increment between elements of vectors in cm[] */
  int icv,        /* Storage increment between vectors in cm[] */
  int ncv         /* Nr of vectors in cm[] to be transformed;
                     if ncv<=0, then no operations will be done on cm[] */
) {
  double d1, d2, b, clinv, cl, sm;
  int incr, k, j, i2, i3, i4;

  /* Check parameters */
  if(mode!=1 && mode!=2) return(1);
  if(m<1 || u==NULL || u_dim1<1 || cm==NULL) return(2);
  if(lpivot<0 || lpivot>=l1 || l1>=m) return(0);
  /* Function Body */
  cl= (d1 = u[lpivot*u_dim1], fabs(d1));
  if(mode==2) { /* Apply transformation I+U*(U**T)/B to cm[] */
    if(cl<=0.) return(0);
  } else { /* Construct the transformation */
    for(j=l1; j<m; j++) { /* Computing MAX */
      d2=(d1=u[j*u_dim1], fabs(d1)); if(d2>cl) cl=d2;}
    if(cl<=0.) return(0);
    clinv=1.0/cl;
    /* Computing 2nd power */
    d1=u[lpivot*u_dim1]*clinv; sm=d1*d1;
    for(j=l1; j<m; j++) {d1=u[j*u_dim1]*clinv; sm+=d1*d1;}
    cl*=sqrt(sm); if(u[lpivot*u_dim1]>0.) cl=-cl;
    *up=u[lpivot*u_dim1]-cl; u[lpivot*u_dim1]=cl;
  }
  if(ncv<=0) return(0);
  b=(*up)*u[lpivot*u_dim1];
  /* b must be nonpositive here; if b>=0., then return */
  if(b>=0.) return(0);
  b=1.0/b; i2=1-icv+ice*lpivot; incr=ice*(l1-lpivot);
  for(j=0; j<ncv; j++) {
    i2+=icv; i3=i2+incr; i4=i3; sm=cm[i2-1]*(*up);
    for(k=l1; k<m; k++) {sm+=cm[i3-1]*u[k*u_dim1]; i3+=ice;}
    if(sm!=0.0) {
      sm*=b; cm[i2-1]+=sm*(*up);
      for(k=l1; k<m; k++) {cm[i4-1]+=sm*u[k*u_dim1]; i4+=ice;}
    }
  }
  return(0);
} /* _nnls_h12 */
/****************************************************************************/

/****************************************************************************/

/****************************************************************************/
/* Local function definitions */
void _nnls_g1_thread(double a, double b, double *cterm, double *sterm, double *sig);
int _nnls_h12_thread(int mode, int lpivot, int l1, int m, double *u, int iue,
              double *up, double *cm, int ice, int icv, int ncv);
/****************************************************************************/

int nnls_threaded(
  double *a, int a_dim1, int m, int n,
  /* On entry, a[] contains the m by n matrix A. On exit, a[] contains the
     product matrix Q*A, where Q is an m by n orthogonal matrix generated
     implicitly by this function. Since matrix A is processed as a set of
     vectors, a_dim1 is needed to specify the storage increment between
     vectors in a[] */
  double *b,
  /* On entry, b[] must contain the m-vector B.
     On exit, b[] contains Q*B */
  double *x,
  /* On exit, x[] will contain the solution vector */
  double *rnorm,
  /* On exit, rnorm contains the Euclidean norm of the residual vector */
  double *wp,  /* An n-array of working space, w[]. */
  /* On exit, w[] will contain the dual solution vector.
     w[i]=0.0 for all i in set p and w[i]<=0.0 for all i in set z. */
  double *zzp, /* An m-array of working space, zz[]. */
  int *indexp  /* An n-array of working space, index[]. */
) {
  int pfeas, ret=0, iz, jz, iz1, iz2, npp1, *index;
  double d1, d2, sm, up, ss, *w, *zz;
  int iter, k, j=0, l, itmax, izmax=0, nsetp, ii, jj=0, ip;
  double temp, wmax, t, alpha, asave, dummy, unorm, ztest, cc;


  /* Check the parameters and data */
  if(m<=0 || n<=0 || a==NULL || b==NULL || x==NULL) return(2);
  /* Allocate memory for working space, if required */
  if(wp!=NULL) w=wp; else w=(double*)calloc(n, sizeof(double));
  if(zzp!=NULL) zz=zzp; else zz=(double*)calloc(m, sizeof(double));
  if(indexp!=NULL) index=indexp; else index=(int*)calloc(n, sizeof(int));
  if(w==NULL || zz==NULL || index==NULL) return(2);

  /* Initialize the arrays INDEX[] and X[] */
  for(k=0; k<n; k++) {x[k]=0.; index[k]=k;}
  iz2=n-1; iz1=0; nsetp=0; npp1=0;

  /* Main loop; quit if all coeffs are already in the solution or */
  /* if M cols of A have been triangularized */
  iter=0; itmax=n*3;
  while(iz1<=iz2 && nsetp<m) {
    /* Compute components of the dual (negative gradient) vector W[] */
    for(iz=iz1; iz<=iz2; iz++) {
      j=index[iz]; sm=0.; for(l=npp1; l<m; l++) sm+=a[l+j*a_dim1]*b[l];
      w[j]=sm;
    }

    while(1) {
      /* Find largest positive W[j] */
      for(wmax=0., iz=iz1; iz<=iz2; iz++) {
        j=index[iz]; if(w[j]>wmax) {wmax=w[j]; izmax=iz;}}

      /* Terminate if wmax<=0.; */
      /* it indicates satisfaction of the Kuhn-Tucker conditions */
      if(wmax<=0.0) break;
      iz=izmax; j=index[iz];

      /* The sign of W[j] is ok for j to be moved to set P. */
      /* Begin the transformation and check new diagonal element to avoid */
      /* near linear dependence. */
      asave=a[npp1+j*a_dim1];
      _nnls_h12_thread(1, npp1, npp1+1, m, &a[j*a_dim1], 1, &up, &dummy, 1, 1, 0);
      unorm=0.;
      if(nsetp!=0) for(l=0; l<nsetp; l++) {d1=a[l+j*a_dim1]; unorm+=d1*d1;}
      unorm=sqrt(unorm);
      d2=unorm+(d1=a[npp1+j*a_dim1], fabs(d1)) * 0.01;
      if((d2-unorm)>0.) {
        /* Col j is sufficiently independent. Copy B into ZZ, update ZZ */
        /* and solve for ztest ( = proposed new value for X[j] ) */
        for(l=0; l<m; l++) zz[l]=b[l];
        _nnls_h12_thread(2, npp1, npp1+1, m, &a[j*a_dim1], 1, &up, zz, 1, 1, 1);
        ztest=zz[npp1]/a[npp1+j*a_dim1];
        /* See if ztest is positive */
        if(ztest>0.) break;
      }

      /* Reject j as a candidate to be moved from set Z to set P. Restore */
      /* A[npp1,j], set W[j]=0., and loop back to test dual coeffs again */
      a[npp1+j*a_dim1]=asave; w[j]=0.;
    } /* while(1) */
    if(wmax<=0.0) break;

    /* Index j=INDEX[iz] has been selected to be moved from set Z to set P. */
    /* Update B and indices, apply householder transformations to cols in */
    /* new set Z, zero subdiagonal elts in col j, set W[j]=0. */
    for(l=0; l<m; ++l) b[l]=zz[l];
    index[iz]=index[iz1]; index[iz1]=j; iz1++; nsetp=npp1+1; npp1++;
    if(iz1<=iz2) for(jz=iz1; jz<=iz2; jz++) {
      jj=index[jz];
      _nnls_h12_thread(2, nsetp-1, npp1, m, &a[j*a_dim1], 1, &up,
           &a[jj*a_dim1], 1, a_dim1, 1);
    }
    if(nsetp!=m) for(l=npp1; l<m; l++) a[l+j*a_dim1]=0.;
    w[j]=0.;
    /* Solve the triangular system; store the solution temporarily in Z[] */
    for(l=0; l<nsetp; l++) {
      ip=nsetp-(l+1);
      if(l!=0) for(ii=0; ii<=ip; ii++) zz[ii]-=a[ii+jj*a_dim1]*zz[ip+1];
      jj=index[ip]; zz[ip]/=a[ip+jj*a_dim1];
    }

    /* Secondary loop begins here */
    while(++iter<itmax) {
      /* See if all new constrained coeffs are feasible; if not, compute alpha */
      for(alpha=2.0, ip=0; ip<nsetp; ip++) {
        l=index[ip];
        if(zz[ip]<=0.) {t=-x[l]/(zz[ip]-x[l]); if(alpha>t) {alpha=t; jj=ip-1;}}
      }

      /* If all new constrained coeffs are feasible then still alpha==2. */
      /* If so, then exit from the secondary loop to main loop */
      if(alpha==2.0) break;
      /* Use alpha (0.<alpha<1.) to interpolate between old X and new ZZ */
      for(ip=0; ip<nsetp; ip++) {l=index[ip]; x[l]+=alpha*(zz[ip]-x[l]);}

      /* Modify A and B and the INDEX arrays to move coefficient i */
      /* from set P to set Z. */
      k=index[jj+1]; pfeas=1;
      do {
        x[k]=0.;
        if(jj!=(nsetp-1)) {
          jj++;
          for(j=jj+1; j<nsetp; j++) {
            ii=index[j]; index[j-1]=ii;
            _nnls_g1_thread(a[j-1+ii*a_dim1], a[j+ii*a_dim1], &cc, &ss, &a[j-1+ii*a_dim1]);
            for(a[j+ii*a_dim1]=0., l=0; l<n; l++) if(l!=ii) {
              /* Apply procedure G2 (CC,SS,A(J-1,L),A(J,L)) */
              temp=a[j-1+l*a_dim1];
              a[j-1+l*a_dim1]=cc*temp+ss*a[j+l*a_dim1];
              a[j+l*a_dim1]=-ss*temp+cc*a[j+l*a_dim1];
            }
            /* Apply procedure G2 (CC,SS,B(J-1),B(J)) */
            temp=b[j-1]; b[j-1]=cc*temp+ss*b[j]; b[j]=-ss*temp+cc*b[j];
          }
        }
        npp1=nsetp-1; nsetp--; iz1--; index[iz1]=k;

        /* See if the remaining coeffs in set P are feasible; they should */
        /* be because of the way alpha was determined. If any are */
        /* infeasible it is due to round-off error. Any that are */
        /* nonpositive will be set to zero and moved from set P to set Z */
        for(jj=0; jj<nsetp; jj++) {k=index[jj]; if(x[k]<=0.) {pfeas=0; break;}}
      } while(pfeas==0);

      /* Copy B[] into zz[], then solve again and loop back */
      for(k=0; k<m; k++) zz[k]=b[k];
      for(l=0; l<nsetp; l++) {
        ip=nsetp-(l+1);
        if(l!=0) for(ii=0; ii<=ip; ii++) zz[ii]-=a[ii+jj*a_dim1]*zz[ip+1];
        jj=index[ip]; zz[ip]/=a[ip+jj*a_dim1];
      }
    } /* end of secondary loop */
    if(iter>itmax) {ret=1; break;}
    for(ip=0; ip<nsetp; ip++) {k=index[ip]; x[k]=zz[ip];}
  } /* end of main loop */
  /* Compute the norm of the final residual vector */
  sm=0.;
  if(npp1<m) for(k=npp1; k<m; k++) sm+=(b[k]*b[k]);
  else for(j=0; j<n; j++) w[j]=0.;
  *rnorm=sqrt(sm);
  /* Free working space, if it was allocated here */
  if(wp==NULL) free(w); if(zzp==NULL) free(zz); if(indexp==NULL) free(index);
  return(ret);
} /* nnls_ */
/****************************************************************************/

/*****************************************************************************
 *
 *  Compute orthogonal rotation matrix:
 *    (C, S) so that (C, S)(A) = (sqrt(A**2+B**2))
 *    (-S,C)         (-S,C)(B)   (   0          )
 *  Compute sig = sqrt(A**2+B**2):
 *    sig is computed last to allow for the possibility that sig may be in
 *    the same location as A or B.
 */
void _nnls_g1_thread(double a, double b, double *cterm, double *sterm, double *sig)
{
  double d1, xr, yr;

  if(fabs(a)>fabs(b)) {
    xr=b/a; d1=xr; yr=sqrt(d1*d1 + 1.); d1=1./yr;
    *cterm=(a>=0.0 ? fabs(d1) : -fabs(d1));
    *sterm=(*cterm)*xr; *sig=fabs(a)*yr;
  } else if(b!=0.) {
    xr=a/b; d1=xr; yr=sqrt(d1*d1 + 1.); d1=1./yr;
    *sterm=(b>=0.0 ? fabs(d1) : -fabs(d1));
    *cterm=(*sterm)*xr; *sig=fabs(b)*yr;
  } else {
    *sig=0.; *cterm=0.; *sterm=1.;
  }
} /* _nnls_g1 */
/****************************************************************************/

/*****************************************************************************
 *
 *  Construction and/or application of a single Householder transformation:
 *           Q = I + U*(U**T)/B
 *
 *  Function returns 0 if succesful, or >0 in case of erroneous parameters.
 *
 */
int _nnls_h12_thread(
  int mode,
  /* mode=1 to construct and apply a Householder transformation, or
     mode=2 to apply a previously constructed transformation */
  int lpivot,     /* Index of the pivot element */
  int l1, int m,
  /* Transformation is constructed to zero elements indexed from l1 to M */
  double *u, int u_dim1, double *up,
  /* With mode=1: On entry, u[] must contain the pivot vector.
     On exit, u[] and up contain quantities defining the vector u[] of
     the Householder transformation. */
  /* With mode=2: On entry, u[] and up should contain quantities previously
     computed with mode=1. These will not be modified. */
  /* u_dim1 is the storage increment between elements. */
  double *cm,
  /* On entry, cm[] must contain the matrix (set of vectors) to which the
     Householder transformation is to be applied. On exit, cm[] will contain
     the set of transformed vectors */
  int ice,        /* Storage increment between elements of vectors in cm[] */
  int icv,        /* Storage increment between vectors in cm[] */
  int ncv         /* Nr of vectors in cm[] to be transformed;
                     if ncv<=0, then no operations will be done on cm[] */
) {
  double d1, d2, b, clinv, cl, sm;
  int incr, k, j, i2, i3, i4;

  /* Check parameters */
  if(mode!=1 && mode!=2) return(1);
  if(m<1 || u==NULL || u_dim1<1 || cm==NULL) return(2);
  if(lpivot<0 || lpivot>=l1 || l1>=m) return(0);
  /* Function Body */
  cl= (d1 = u[lpivot*u_dim1], fabs(d1));
  if(mode==2) { /* Apply transformation I+U*(U**T)/B to cm[] */
    if(cl<=0.) return(0);
  } else { /* Construct the transformation */
    for(j=l1; j<m; j++) { /* Computing MAX */
      d2=(d1=u[j*u_dim1], fabs(d1)); if(d2>cl) cl=d2;}
    if(cl<=0.) return(0);
    clinv=1.0/cl;
    /* Computing 2nd power */
    d1=u[lpivot*u_dim1]*clinv; sm=d1*d1;
    for(j=l1; j<m; j++) {d1=u[j*u_dim1]*clinv; sm+=d1*d1;}
    cl*=sqrt(sm); if(u[lpivot*u_dim1]>0.) cl=-cl;
    *up=u[lpivot*u_dim1]-cl; u[lpivot*u_dim1]=cl;
  }
  if(ncv<=0) return(0);
  b=(*up)*u[lpivot*u_dim1];
  /* b must be nonpositive here; if b>=0., then return */
  if(b>=0.) return(0);
  b=1.0/b; i2=1-icv+ice*lpivot; incr=ice*(l1-lpivot);
  for(j=0; j<ncv; j++) {
    i2+=icv; i3=i2+incr; i4=i3; sm=cm[i2-1]*(*up);
    for(k=l1; k<m; k++) {sm+=cm[i3-1]*u[k*u_dim1]; i3+=ice;}
    if(sm!=0.0) {
      sm*=b; cm[i2-1]+=sm*(*up);
      for(k=l1; k<m; k++) {cm[i4-1]+=sm*u[k*u_dim1]; i4+=ice;}
    }
  }
  return(0);
} /* _nnls_h12_thread */
/****************************************************************************/

/****************************************************************************/

