#include "../include/us_saxs_util.h"

#if defined(CUDA)

#if defined(USE_MPI)
#include <mpi.h>
extern int npes;
extern int myrank;
#endif

#include "../us_saxs_cmds_mpi_cuda/us_cuda.h"

bool US_Saxs_Util::iqq_cuda(vector<double> &q, vector<saxs_atom> &atoms,
                            vector<vector<double> > &fp, vector<double> &I,
                            unsigned int threads_per_block) {
  if (!our_saxs_options.autocorrelate) {
    errormsg = "cuda iqq does not currently support no autocorrelation";
    return false;
  }

  if (our_saxs_options.subtract_radius) {
    errormsg = "cuda iqq does not currently support radius subtraction option";
    return false;
  }

  vector<float> f_fp;
  vector<float> f_q;
  vector<float> f_pos;
  vector<float> f_I(I.size());

  for (unsigned int i = 0; i < q.size(); ++i) {
    f_q.push_back((float)q[i]);
  }

  for (unsigned int i = 0; i < atoms.size(); ++i) {
    f_pos.push_back(atoms[i].pos[0]);
    f_pos.push_back(atoms[i].pos[1]);
    f_pos.push_back(atoms[i].pos[2]);
  }

  for (unsigned int i = 0; i < fp.size(); ++i)  // should be q.size()
  {
    for (unsigned int j = 0; j < fp[i].size(); ++j)  // should be atoms.size()
    {
      f_fp.push_back(fp[i][j]);
    }
  }

  cuda_debye(our_saxs_options.autocorrelate, atoms.size(), &(f_pos[0]),
             f_q.size(), &(f_q[0]), &(f_fp[0]), &(f_I[0]), threads_per_block
#if defined(USE_MPI)
             ,
             myrank
#endif
  );

  for (unsigned int i = 0; i < I.size(); i++) {
    I[i] = (double)f_I[i];
  }
  return true;
}

#else
bool US_Saxs_Util::iqq_cuda(vector<double> &, vector<saxs_atom> &,
                            vector<vector<double> > &, vector<double> &,
                            unsigned int) {
  errormsg = "Cuda not supported in this version";
  return false;
}

#endif
