#include "../include/us_efa.h"

#include "../include/us_mqt.h"
#include "../include/us_timer.h"
#include "../include/us_vector.h"

// #define DEBUG_THREAD
#define SHOW_TIMINGS

bool US_Efa::Efa(vector<vector<double> > &A, vector<vector<double> > &result,
                 bool rightEfa) {
  qDebug() << "US_Efa::Efa";
  errors = "";

  result.clear();

  // check A
  // go thru a column at a time
  // invert svd if needed ?
  // incrementally build up result
  // print out results
  // check A
  // build up double ** column at a time ?

  // m rows
  // n cols

#if defined(SHOW_TIMINGS)
  US_Timer us_timers;
  us_timers.clear_timers();
  us_timers.init_timer("EFA 0 setup");
  us_timers.init_timer("EFA 1 SVD");
  us_timers.init_timer("EFA 2 post");
  us_timers.init_timer("EFA 3 printout");
  us_timers.init_timer("EFA 9 total");

  us_timers.start_timer("EFA 9 total");
#endif

  int n = (int)A.size();
  if (!n) {
    errors = "US_Efa::Efa matrix A has no columns";
    return false;
  }

#if defined(SHOW_TIMINGS) || defined(DO_PRINTOUT)
  QTextStream ts(stdout);
#endif

  int m = (int)A[0].size();

  result.resize(n);

  // for each column
  for (int n_use = 0; n_use < n; ++n_use) {
#if defined(SHOW_TIMINGS)
    us_timers.start_timer("EFA 0 setup");
#endif
    int n_usep1 = n_use + 1;
    vector<double *> a(m);
    // build up a
    vector<vector<double> > F(m);
    if (rightEfa) {
      for (int i = 0; i < m; ++i) {
        F[i].resize(n_usep1);
        for (int j = 0; j < n_usep1; ++j) {
          F[i][j] = A[n - j - 1][i];
        }
        a[i] = &(F[i][0]);
      }
    } else {
      for (int i = 0; i < m; ++i) {
        F[i].resize(n_usep1);
        for (int j = 0; j < n_usep1; ++j) {
          F[i][j] = A[j][i];
        }
        a[i] = &(F[i][0]);
      }
    }

    vector<double> W(n_usep1);
    double *w = &(W[0]);

    vector<double *> v(n_usep1);
    vector<vector<double> > V(n_usep1);
    for (int j = 0; j < n_usep1; ++j) {
      V[j].resize(n_usep1);
      v[j] = &(V[j][0]);
    }

#if defined(SHOW_TIMINGS)
    us_timers.end_timer("EFA 0 setup");
    us_timers.start_timer("EFA 1 SVD");
#endif
    // SVD::dinfo( &(a[ 0 ]), m, n_usep1, &(w[ 0 ]), &(v[ 0 ]) );
    // SVD::dvalidate( &(a[ 0 ]), m, n_usep1, &(w[ 0 ]), &(v[ 0 ]) );

    if (!SVD::dsvd(&(a[0]), m, n_usep1, &(w[0]), &(v[0]))) {
      errors = SVD::errormsg;
      return false;
    }
#if defined(SHOW_TIMINGS)
    us_timers.end_timer("EFA 1 SVD");
    us_timers.start_timer("EFA 2 post");
#endif
    list<svd_sortable_double> svals;

    svd_sortable_double sval;
    for (int i = 0; i < n_usep1; i++) {
      sval.x = w[i];
      sval.index = i;
      svals.push_back(sval);
    }
    svals.sort();
    svals.reverse();

    result[n_use].clear();

    for (list<svd_sortable_double>::iterator it = svals.begin();
         it != svals.end(); ++it) {
      result[n_use].push_back(it->x);
    }
#if defined(SHOW_TIMINGS)
    us_timers.end_timer("EFA 2 post");
    us_timers.start_timer("EFA 3 printout");
#endif
#if defined(DO_PRINTOUT)
    {
      vector<double> rpt = result[n_use];
      rpt.resize(5);

      ts << (rightEfa ? n - n_use - 1 : n_use) << " " << rpt[0] << " " << rpt[1]
         << " " << rpt[2] << " " << rpt[3] << " " << rpt[4] << "\n";
      // US_Vector::printvector( QString( "US_Efa::Efa result[ %1 ]" ).arg(
      // n_use ), rpt );
    }
#endif
#if defined(SHOW_TIMINGS)
    us_timers.end_timer("EFA 3 printout");
#endif
  }

#if defined(SHOW_TIMINGS)
  us_timers.end_timer("EFA 9 total");
  ts << us_timers.list_times();
#endif
  return true;
}

bool US_Efa::Efa_t(unsigned int numThreads, vector<vector<double> > &A,
                   vector<vector<double> > &result, bool rightEfa) {
  qDebug() << "US_Efa::Efa_t";
  errors = "";

  result.clear();

  // check A
  // go thru a column at a time
  // invert svd if needed ?
  // incrementally build up result
  // print out results
  // check A
  // build up double ** column at a time ?

  // m rows
  // n cols

#if defined(SHOW_TIMINGS)
  US_Timer us_timers;
  us_timers.clear_timers();
  us_timers.init_timer("EFA 0 setup");
  us_timers.init_timer("EFA 1 SVD");
  us_timers.init_timer("EFA 2 post");
  us_timers.init_timer("EFA 3 printout");
  us_timers.init_timer("EFA 9 total");

  us_timers.start_timer("EFA 9 total");
#endif

  int n = (int)A.size();
  if (!n) {
    errors = "US_Efa::Efa matrix A has no columns";
    return false;
  }

#if defined(SHOW_TIMINGS) || defined(DO_PRINTOUT)
  QTextStream ts(stdout);
#endif

  int m = (int)A[0].size();

  result.resize(n);

  // for each column
  // need to preallocate all ...

  vector<double **> a_t(n);
  vector<int> n_t(n);
  vector<double *> w_t(n);
  vector<double **> v_t(n);
  vector<QString> errors_t(n);

  vector<vector<double *> > aa_t(n);
  vector<vector<vector<double> > > F_t(n);
  vector<vector<double> > W_t(n);
  vector<vector<double *> > vv_t(n);
  vector<vector<vector<double> > > V_t(n);

#if defined(SHOW_TIMINGS)
  us_timers.start_timer("EFA 0 setup");
#endif

  for (int n_use = 0; n_use < n; ++n_use) {
    int n_usep1 = n_use + 1;
    n_t[n_use] = n_usep1;
    aa_t[n_use].resize(m);
    // build up a
    F_t[n_use].resize(m);
    if (rightEfa) {
      for (int i = 0; i < m; ++i) {
        F_t[n_use][i].resize(n_usep1);
        for (int j = 0; j < n_usep1; ++j) {
          F_t[n_use][i][j] = A[n - j - 1][i];
        }
        aa_t[n_use][i] = &(F_t[n_use][i][0]);
      }
    } else {
      for (int i = 0; i < m; ++i) {
        F_t[n_use][i].resize(n_usep1);
        for (int j = 0; j < n_usep1; ++j) {
          F_t[n_use][i][j] = A[j][i];
        }
        aa_t[n_use][i] = &(F_t[n_use][i][0]);
      }
    }
    a_t[n_use] = &(aa_t[n_use][0]);

    W_t[n_use].resize(n_usep1);
    w_t[n_use] = &(W_t[n_use][0]);

    // v needs work, verify all
    vv_t[n_use].resize(n_usep1);
    V_t[n_use].resize(n_usep1);
    for (int j = 0; j < n_usep1; ++j) {
      V_t[n_use][j].resize(n_usep1);
      vv_t[n_use][j] = &(V_t[n_use][j][0]);
    }
    v_t[n_use] = &(vv_t[n_use][0]);

    // SVD::dinfo( a_t[ n_use ], m, n_usep1, w_t[ n_use ], v_t[ n_use ] );

    // if ( !SVD::dsvd( &(a[ 0 ]), m, n_use, &(w[ 0 ]), &(v[ 0 ]) ) ) {
    //    errors = SVD::errormsg;
    //    return false;
    // }
  }
#if defined(SHOW_TIMINGS)
  us_timers.end_timer("EFA 0 setup");

  us_timers.start_timer("EFA 1 SVD");
#endif
  // do threaded SVD

  {
    // threaded

    unsigned int j;
    unsigned int threads = numThreads;
    qDebug() << QString("US_Efa_t:Using %1 threads.\n").arg(threads);
    vector<efa_thr_t *> efa_thr_threads(threads);
    for (j = 0; j < threads; j++) {
      efa_thr_threads[j] = new efa_thr_t(j);
      efa_thr_threads[j]->start();
    }

    for (j = 0; j < threads; j++) {
#if defined(DEBUG_THREAD)
      qDebug() << "thread " << j << endl;
#endif
      efa_thr_threads[j]->efa_thr_setup(a_t, m, n_t, w_t, v_t, threads,
                                        errors_t  //,
                                        // progress,
                                        // lbl_core_progress,
                                        // stopFlag
      );
    }
    // sleep app loop
    {
      int all_done;
      do {
        all_done = threads;
        for (j = 0; j < threads; j++) {
          all_done -= efa_thr_threads[j]->efa_thr_work_status();
        }
        qApp->processEvents();
        mQThread::msleep(333);
      } while (all_done);
    }

    // wait for work to complete

    for (j = 0; j < threads; j++) {
      efa_thr_threads[j]->efa_thr_wait();
    }

    // destroy

    for (j = 0; j < threads; j++) {
      efa_thr_threads[j]->efa_thr_shutdown();
    }

    for (j = 0; j < threads; j++) {
      efa_thr_threads[j]->wait();
    }

    for (j = 0; j < threads; j++) {
      delete efa_thr_threads[j];
    }
  }  // end threaded

#if defined(SHOW_TIMINGS)
  us_timers.end_timer("EFA 1 SVD");
#endif
  // collect results

#if defined(SHOW_TIMINGS)
  us_timers.start_timer("EFA 2 post");
#endif

  for (int n_use = 0; n_use < n; ++n_use) {
    int n_usep1 = n_use + 1;
    if (!errors_t[n_use].isEmpty()) {
      errors = errors_t[n_use];
      return false;
    }

    list<svd_sortable_double> svals;

    svd_sortable_double sval;
    for (int i = 0; i < n_usep1; i++) {
      sval.x = w_t[n_use][i];
      sval.index = i;
      svals.push_back(sval);
    }
    svals.sort();
    svals.reverse();

    result[n_use].clear();

    for (list<svd_sortable_double>::iterator it = svals.begin();
         it != svals.end(); ++it) {
      result[n_use].push_back(it->x);
    }
  }

#if defined(SHOW_TIMINGS)
  us_timers.end_timer("EFA 2 post");

  us_timers.start_timer("EFA 3 printout");
#endif
#if defined(DO_PRINTOUT)
  for (int n_use = 0; n_use < n; ++n_use) {
    vector<double> rpt = result[n_use];
    rpt.resize(5);

    ts << (rightEfa ? n - n_use - 1 : n_use) << " " << rpt[0] << " " << rpt[1]
       << " " << rpt[2] << " " << rpt[3] << " " << rpt[4] << "\n";
    // US_Vector::printvector( QString( "US_Efa::Efa result[ %1 ]" ).arg( n_use
    // ), rpt );
  }
#endif
#if defined(SHOW_TIMINGS)
  us_timers.end_timer("EFA 3 printout");
  us_timers.end_timer("EFA 9 total");

  ts << us_timers.list_times();
#endif
  return true;
}

efa_thr_t::efa_thr_t(int a_thread) : QThread() {
  thread = a_thread;
  work_to_do = 0;
  work_done = 1;
  work_to_do_waiters = 0;
  work_done_waiters = 0;
}

void efa_thr_t::efa_thr_setup(vector<double **> &a_t, int m, vector<int> &n_t,
                              vector<double *> &w_t, vector<double **> &v_t,
                              unsigned int threads,
                              vector<QString> &errors_t  //,

                              // QProgressBar *progress,
                              // QLabel *lbl_core_progress,
                              // bool *stopFlag
) {
  /* this starts up a new work load for the thread */

  this->a_t = &a_t;
  this->m = m;
  this->n_t = &n_t;
  this->w_t = &w_t;
  this->v_t = &v_t;
  this->threads = threads;
  this->errors_t = &errors_t;

  // this->progress = progress;
  // this->lbl_core_progress = lbl_core_progress;
  // this->stopFlag = stopFlag;

  work_mutex.lock();
  work_to_do = 1;
  work_done = 0;
  work_mutex.unlock();
  cond_work_to_do.wakeOne();
#if defined(DEBUG_THREAD)
  qDebug() << "thread " << thread << " has new work to do\n";
#endif
}

void efa_thr_t::efa_thr_shutdown() {
  /* this signals the thread to exit the run method */
  work_mutex.lock();
  work_to_do = -1;
  work_mutex.unlock();
  cond_work_to_do.wakeOne();

#if defined(DEBUG_THREAD)
  qDebug() << "thread " << thread << " shutdown requested\n";
#endif
}

void efa_thr_t::efa_thr_wait() {
  /* this is for the master thread to wait until the work is done */
  work_mutex.lock();

#if defined(DEBUG_THREAD)
  qDebug() << "thread " << thread << " has a waiter\n";
#endif

  while (!work_done) {
    cond_work_done.wait(&work_mutex);
  }
  work_done = 0;
  work_mutex.unlock();

#if defined(DEBUG_THREAD)
  qDebug() << "thread " << thread << " waiter released\n";
#endif
}

int efa_thr_t::efa_thr_work_status() {
  work_mutex.lock();
  int retval = work_done;
  work_mutex.unlock();
  return retval;
}

void efa_thr_t::run() {
  while (1) {
    work_mutex.lock();
#if defined(DEBUG_THREAD)
    qDebug() << "thread " << thread << " waiting for work\n";
#endif
    work_to_do_waiters++;
    while (!work_to_do) {
      cond_work_to_do.wait(&work_mutex);
    }
    if (work_to_do == -1) {
#if defined(DEBUG_THREAD)
      qDebug() << "thread " << thread << " shutting down\n";
#endif
      work_mutex.unlock();
      return;
    }

    work_to_do_waiters = 0;
    work_mutex.unlock();
#if defined(DEBUG_THREAD)
    qDebug() << "thread " << thread << " starting work\n";
#endif

    // do work

    unsigned int jobs = (unsigned int)(*a_t).size();

    // if ( !thread )  {
    //    progress->setMaximum((int)(1.15f * as1 / threads));
    // }
#if defined(DEBUG_THREAD)
    qDebug() << "thread " << thread << " jobs = " << jobs << endl;

#endif

    for (unsigned int i = thread; i < jobs; i += threads) {
#if defined(DEBUG_THREAD)
      qDebug() << "thread " << thread << " i = " << i << " m = " << m
               << " n = " << (*n_t)[i] << endl;
#endif
      // if ( !thread ) {
      //    // lbl_core_progress->setText(QString("Atom %1 of
      //    %2\n").arg(i+1).arg(as)); progress->setValue(i+1);
      //    // qApp->processEvents();
      // }
      // if ( *stopFlag ) {
      //    break;
      // }

      // SVD::dinfo( (*a_t)[ i ], m, (*n_t)[ i ], (*w_t)[ i ], (*v_t)[ i ] );
      // SVD::dvalidate( (*a_t)[ i ], m, (*n_t)[ i ], (*w_t)[ i ], (*v_t)[ i ]
      // );

      if (!SVD::dsvd((*a_t)[i], m, (*n_t)[i], (*w_t)[i], (*v_t)[i])) {
        (*errors_t)[i] = QString("%1").arg(SVD::errormsg);
      }
    }

#if defined(DEBUG_THREAD)
    qDebug() << "thread " << thread << " finished work\n";
#endif
    work_mutex.lock();
    work_done = 1;
    work_to_do = 0;
    work_mutex.unlock();
    cond_work_done.wakeOne();
  }
}
