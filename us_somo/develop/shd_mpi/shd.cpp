#include "shd.h"

SHD::SHD(unsigned int max_harmonics, vector<shd_point> &model,
         vector<vector<shd_double> > &F, vector<shd_double> &q,
         vector<shd_double> &I, int debug_level) {
  this->max_harmonics = max_harmonics;
  this->model = &model;
  this->F = F;
  this->q = q;
  this->I = I;
  this->debug_level = debug_level;

  q_points = (unsigned int)q.size();

  i_ = complex<float>(0.0f, 1.0f);

  i_l.resize(max_harmonics + 1);
  for (unsigned int l = 0; l <= max_harmonics; l++) {
    i_l[l] = pow(i_, l);
  }

  Y_points = max_harmonics + 1 + (max_harmonics) * (max_harmonics + 1);
  J_points = (1 + max_harmonics) * q_points;
  i_k.clear();
  for (unsigned int l = 0; l <= max_harmonics; ++l) {
    for (int m = -(int)l; m <= (int)l; m++) {
      i_k.push_back(i_l[l]);
    }
  }

  no_harmonics = max_harmonics + 1;

  q_Y_points = q_points * Y_points;

  {
    complex<float> Z0 = complex<float>(0.0f, 0.0f);
    for (unsigned int i = 0; i < q_Y_points; ++i) {
      A1v0.push_back(Z0);
    }
  }

  //    if ( !world_rank )
  //    {
  //       printf( "q_Y_points %d\n", q_Y_points );
  //       printf( "A1v0 size  %d\n", (int)A1v0.size() );
  //    }

  ccY.resize(Y_points);
  ccJ.resize(J_points);
  ccA1v = A1v0;

  shs = new SHS_USE(max_harmonics);
}

SHD::~SHD() {}

void SHD::printmodel() {
  printf("%d: model (%d):\n", world_rank, (int)model->size());
  for (int i = 0; i < (int)model->size(); i++) {
    printf("%d: %f %f %f %d\n", world_rank, (*model)[i].x[0], (*model)[i].x[1],
           (*model)[i].x[2], (*model)[i].ff_type);
  }
}

void SHD::printF() {
  printf("%d: F (%d):\n", world_rank, (int)F.size());
  for (int i = 0; i < (int)F.size(); i++) {
    printf("%d: %d (%d):", world_rank, i, (int)F[i].size());
    for (int j = 0; j < (int)F[i].size(); j++) {
      printf(" %f ", F[i][j]);
    }
    printf("\n");
  }
}

void SHD::printq() {
  printf("%d: q (%d):", world_rank, (int)q.size());
  for (int i = 0; i < (int)q.size(); i++) {
    printf(" %f", q[i]);
  }
  printf("\n");
}

void SHD::printA(vector<complex<float> > &A) {
  printf("amplitudes\n");
  for (int i = 0; i < (int)A.size(); i++) {
    cout << A[i] << endl;
  }
  printf("\n");
}
