
#ifndef US_THREAD_H
#define US_THREAD_H

#include <iostream>
#ifdef THREAD
#include <qthread.h>

class CATA_D_Thread : public QThread {
 public:
  double ***product, ***A;
  unsigned int i, j, k, end_j, end_k;
  virtual void run();
};

class CATA_F_Thread : public QThread {
 public:
  float ***product, ***A;
  unsigned int i, j, k, end_j, end_k;
  virtual void run();
};
#endif
#endif
