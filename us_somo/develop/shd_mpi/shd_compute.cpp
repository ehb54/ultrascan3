#include "shd.h"

// #define SHOW_MPI_TIMING

bool SHD::compute_amplitudes(vector<complex<float> > &Av) {
  // model should already be centered
#if defined(SHOW_MPI_TIMING)
  double global_time_start = MPI_Wtime();
  double time_start;
  double time_end;
  double rtp_time = 0e0;
  double shbes_time = 0e0;
  double legendre_time = 0e0;
#endif

  shd_data tmp_data;
  shd_data *datap = &tmp_data;
  tmp_data.A1v = A1v0;

  complex<float> *Yp;

  shd_double *Jp;
  shd_double *qp;
  shd_double *Fp;
  shd_double qp_t_rtp0;

  complex<shd_double> tmp_cd;

  complex<float> *i_lp;
  // complex < float > *Ap;
  complex<float> *A1vp;
  complex<float> tmp_cf;

  Av = A1v0;

  shd_point *modelp = (shd_point *)(&((*model)[0]));
  int model_size = (int)model->size();

  for (int m = 0; m < model_size; ++m, ++modelp) {
    if (modelp->ff_type == -1) {
      break;
    }

#if defined(SHOW_MPI_TIMING)
    time_start = MPI_Wtime();
#endif
    datap->rtp[0] = sqrt((shd_double)(modelp->x[0] * modelp->x[0] +
                                      modelp->x[1] * modelp->x[1] +
                                      modelp->x[2] * modelp->x[2]));
    if (datap->rtp[0] == 0e0) {
      datap->rtp[1] = 0e0;
      datap->rtp[2] = 0e0;
    } else {
      datap->rtp[1] = acos(modelp->x[2] / datap->rtp[0]);

      if (modelp->x[0] == 0 && modelp->x[1] == 0) {
        datap->rtp[2] = 0e0;
      } else {
        shd_double asinc =
            modelp->x[1] /
            sqrt((modelp->x[0] * modelp->x[0] + modelp->x[1] * modelp->x[1]));
        if (asinc > 1e0) {
          asinc = 1e0;
        } else {
          if (asinc < -1e0) {
            asinc = -1e0;
          }
        }

        // int last_case = -1;
        if (modelp->x[0] < 0) {
          // last_case = 1;
          datap->rtp[2] = M_PI - asin(asinc);
        } else {
          if (modelp->x[1] < 0) {
            // last_case = 2;
            datap->rtp[2] = M_2PI + asin(asinc);
          } else {
            // last_case = 3;
            datap->rtp[2] = asin(asinc);
          }
        }

        // if ( isnan( datap->rtp[ 2 ] ) )
        // {
        //    error_msg = "SHD::compute_amplitudes: error phi isnan";
        //    cout << "last case: " << last_case << endl;
        //    cout << "rtp   " << datap->rtp[ 0 ] << " " << datap->rtp[ 1 ] << "
        //    " << datap->rtp[ 2 ] << endl; cout << "coord " << modelp->x[ 0 ]
        //    << " " << modelp->x[ 1 ] << " " << modelp->x[ 2 ] << endl; cout <<
        //    "x^2 + y^2 " << ( modelp->x[ 0 ] *
        //                              modelp->x[ 0 ] +
        //                              modelp->x[ 1 ] *
        //                              modelp->x[ 1 ] ) << endl;

        //    cout << "y / x^2 + y^2 " << ( modelp->x[ 1 ] / sqrt( modelp->x[ 0
        //    ] *
        //                                                         modelp->x[ 0
        //                                                         ] +
        //                                                         modelp->x[ 1
        //                                                         ] *
        //                                                         modelp->x[ 1
        //                                                         ] ) ) <<
        //                                                         endl;

        //    cout << "y / x^2 + y^2 " << ( modelp->x[ 1 ] / sqrt( (shd_double)
        //    ( modelp->x[ 0 ] *
        //                                                                        modelp->x[ 0 ] +
        //                                                                        modelp->x[ 1 ] *
        //                                                                        modelp->x[ 1 ] ) ) ) << endl;

        //    cout << "asin( y / x^2 + y^2 ) " << asin( ( modelp->x[ 1 ] / sqrt(
        //    modelp->x[ 0 ] *
        //                                                                       modelp->x[ 0 ] +
        //                                                                       modelp->x[ 1 ] *
        //                                                                       modelp->x[ 1 ] ) ) ) << endl;

        //    double tmp1 = asin( ( modelp->x[ 1 ] / sqrt( modelp->x[ 0 ] *
        //                                                 modelp->x[ 0 ] +
        //                                                 modelp->x[ 1 ] *
        //                                                 modelp->x[ 1 ] ) ) );
        //    double tmp2 = asin( ( modelp->x[ 1 ] / sqrt( (shd_double) (
        //    modelp->x[ 0 ] *
        //                                                                modelp->x[
        //                                                                0 ] +
        //                                                                modelp->x[
        //                                                                1 ] *
        //                                                                modelp->x[
        //                                                                1 ] )
        //                                                                ) ) );
        //    cout << "tmp1 " << tmp1 << endl;
        //    cout << "tmp2 " << tmp2 << endl;
        //    cout << "M_PI - tmp1 " << M_PI - tmp1 << endl;
        //    cout << "M_PI - tmp2 " << M_PI - tmp2 << endl;

        //    cout << "M_PI " << M_PI << endl;

        //    cout << "direct recompute: " << (M_PI - asin( ( modelp->x[ 1 ] /
        //    sqrt( (shd_double) ( modelp->x[ 0 ] *
        //                                                                                          modelp->x[ 0 ] +
        //                                                                                          modelp->x[ 1 ] *
        //                                                                                          modelp->x[ 1 ] ) ) ) ) ) << endl;

        //    cout << "direct recompute 2: " << ((shd_double)M_PI -
        //    (shd_double)asin( ( modelp->x[ 1 ] / sqrt( (shd_double) (
        //    modelp->x[ 0 ] *
        //                                                                                                                    modelp->x[ 0 ] +
        //                                                                                                                    modelp->x[ 1 ] *
        //                                                                                                                    modelp->x[ 1 ] ) ) ) ) ) << endl;
        //    return false;
        // }
      }
    }
#if defined(SHOW_MPI_TIMING)
    time_end = MPI_Wtime();
    rtp_time += time_end - time_start;
#endif

    Yp = &(ccY[0]);

#if defined(SHOW_MPI_TIMING)
    time_start = MPI_Wtime();
#endif
#define ALT_SH
#if defined(ALT_SH)
    sh::alt_conj_sh(max_harmonics, datap->rtp[1], datap->rtp[2], Yp);
    // for ( unsigned int l = 0; l <= max_harmonics; ++l )
    // {
    //    if ( isnan( real( Yp[ l ] ) ) ||
    //         isnan( imag( Yp[ l ] ) ) )
    //    {
    //       error_msg = "SHD::compute_amplitudes: error Y isnan";
    //       cout << "rtp   " << datap->rtp[ 0 ] << " " << datap->rtp[ 1 ] << "
    //       " << datap->rtp[ 2 ] << endl; cout << "coord " << modelp->x[ 0 ] <<
    //       " " << modelp->x[ 1 ] << " " << modelp->x[ 2 ] << endl; return
    //       false;
    //    }
    // }
#else
    for (unsigned int l = 0; l <= max_harmonics; ++l) {
      for (int m = -(int)l; m <= (int)l; ++m) {
        if (!sh::conj_spherical_harmonic(l, m, datap->rtp[1], datap->rtp[2],
                                         tmp_cd)) {
          error_msg = "sh::spherical_harmonic failed";
          return false;
        }

        (*Yp) = tmp_cd;
        ++Yp;
      }
    }
#endif
#if defined(SHOW_MPI_TIMING)
    time_end = MPI_Wtime();
    legendre_time += time_end - time_start;
#endif

    qp = &(q[0]);
    Fp = &(F[modelp->ff_type][0]);
    A1vp = &(Av[0]);
    // Ap   = &( datap->A1v[ 0 ] );

    for (unsigned int j = 0; j < q_points; ++j) {
      qp_t_rtp0 = (*qp) * datap->rtp[0];
      ++qp;

      i_lp = &(i_l[0]);
      Yp = &(ccY[0]);
      Jp = &(ccJ[0]);

#define ALT_SPHBES
#if defined(ALT_SPHBES)
#if defined(SHOW_MPI_TIMING)
      time_start = MPI_Wtime();
#endif
      if (!shs->shs_compute_sphbes(qp_t_rtp0, Jp)) {
        error_msg = "nr::alt_shbes failed";
        return false;
      }
#if defined(SHOW_MPI_TIMING)
      time_end = MPI_Wtime();
      shbes_time += time_end - time_start;
#endif

      for (unsigned int l = 0; l <= max_harmonics; ++l) {
        tmp_cf = (float)*Jp * (float)(*Fp) * (*i_lp);
        for (int m = -(int)l; m <= (int)l; ++m) {
          (*A1vp) += (*Yp) * tmp_cf;  // (*Ap);
          ++Yp;
          ++A1vp;
        }
        ++Jp;
        ++i_lp;
      }
#else
      for (unsigned int l = 0; l <= max_harmonics; ++l) {
#if defined(SHOW_MPI_TIMING)
        time_start = MPI_Wtime();
#endif
        if (!nr::sphbes(l, qp_t_rtp0, *Jp)) {
          error_msg = "nr::shbes failed";
          return false;
        }
#if defined(SHOW_MPI_TIMING)
        time_end = MPI_Wtime();
        shbes_time += time_end - time_start;
#endif

        tmp_cf = (float)*Jp * (float)(*Fp) * (*i_lp);
        for (int m = -(int)l; m <= (int)l; ++m) {
          (*A1vp) += (*Yp) * tmp_cf;  // (*Ap);
          ++Yp;
          ++A1vp;
        }
        // ++Jp;
        ++i_lp;
      }
#endif
      ++Fp;
    }
  }
#if defined(SHOW_MPI_TIMING)
  time_end = MPI_Wtime();
  double total_time = time_end - global_time_start;

  printf(
      "%d of %d: compute amplitudes model size %d rtp %gms %.2f leg %gms %.2f "
      "shbes %gms %.2f other %gms %.2f tot %gms\n",
      world_rank, world_size, (int)model->size(), rtp_time * 1e3,
      1e2 * (rtp_time) / total_time, legendre_time * 1e3,
      1e2 * (legendre_time) / total_time, shbes_time * 1e3,
      1e2 * (shbes_time) / total_time,
      (total_time - rtp_time - legendre_time - shbes_time) * 1e3,
      1e2 * (total_time - rtp_time - legendre_time - shbes_time) / total_time,
      total_time * 1e3);
#endif

  return true;
}
