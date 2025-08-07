#ifndef US_EFA_H
#define US_EFA_H

#include <QtCore>

#include <list>
#include <vector>
#include "../include/us_svd.h"

using namespace std;

class US_Efa {
   public:
      bool Efa(vector<vector<double>> &A, vector<vector<double>> &result, bool rightEfa = false);
      bool Efa_t(
         unsigned int numThreads, vector<vector<double>> &A, vector<vector<double>> &result, bool rightEfa = false);
      QString errors;

   private:
};

class efa_thr_t : public QThread {
   public:
      /* constructor */ efa_thr_t(int);
      void efa_thr_setup(
         vector<double **> &a_t, int m, vector<int> &n_t, vector<double *> &w_t, vector<double **> &v_t,
         unsigned int threads,
         vector<QString> &errors_t //,
         // QProgressBar *progress,
         // QLabel *lbl_core_progress,
         // bool *stopFlag
      );
      void efa_thr_shutdown();
      void efa_thr_wait();
      int efa_thr_work_status();
      virtual void run();

   private:
      vector<double **> *a_t;
      int m;
      vector<int> *n_t;
      vector<double *> *w_t;
      vector<double **> *v_t;
      unsigned int threads;
      vector<QString> *errors_t;

      // QProgressBar *progress;
      // QLabel *lbl_core_progress;
      // bool *stopFlag;

      int thread;
      QMutex work_mutex;
      int work_to_do;
      QWaitCondition cond_work_to_do;
      int work_done;
      QWaitCondition cond_work_done;
      int work_to_do_waiters;
      int work_done_waiters;
};

#endif
