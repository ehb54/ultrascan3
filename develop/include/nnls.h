/******************************************************************************
  Copyright (c) 2002 by Turku PET Centre

  nnls.h
  
  Version:
  2002-08-19 Vesa Oikonen

******************************************************************************/
#ifndef _NNLS_H
#define _NNLS_H
/*****************************************************************************/
int nnls(double *a, int a_dim1, int m, int n, double *b, double *x,
          double *rnorm, double *w, double *zz, int *index);
int nnls_thread(double *a, int a_dim1, int m, int n, double *b, double *x,
                double *rnorm, double *w, double *zz, int *index);
/*****************************************************************************/
#endif

