#ifdef THREAD
#include "../include/us_thread.h"

void CATA_D_Thread::run() {
  for (j = 0; j < end_j; j++) {
    (*product)[i][j] = 0.0;
    for (k = 0; k < end_k; k++) {
      (*product)[i][j] += (*A)[k][i] * (*A)[k][j];
    }
  }
}

void CATA_F_Thread::run() {
  for (j = 0; j < end_j; j++) {
    (*product)[i][j] = 0.0;
    for (k = 0; k < end_k; k++) {
      (*product)[i][j] += (*A)[k][i] * (*A)[k][j];
    }
  }
}

#endif
