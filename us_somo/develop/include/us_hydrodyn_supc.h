#ifndef US_HYDRODYN_SUPC_H
#define US_HYDRODYN_SUPC_H

struct dati1_supc
{
   float x, y, z;      /* coordinates of the bead center    */
   float r;         /* hydrated radius of the bead       */
   float ru;         /* unhydrated radius of the bead     */
   float m;         /* mass of the bead                  */
   int col;         /* color of the bead                 */
   char *cor;         /* correspondence between beads and AA */
};

#include <qlistwidget.h>

#include <stdio.h>
#include <math.h>
#ifdef OSX
#include <sys/malloc.h>
#endif
#include <string.h>

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_hydro.h"
#include "../include/us_hydrodyn_pdbdefs.h"


int
us_hydrodyn_supc_main(hydro_results *hydro_results, 
                      hydro_options *hydro, 
                      double use_overlap_tolerance,
                      vector < vector <PDB_atom> > *bead_models, 
                      vector <int> *somo_processed,
                      vector <PDB_model> *model_vector,
                      QListWidget *lb_model,
                      const char *filename,
                      const char *res_filename,
                      vector < QString > model_names,
                      QProgressBar *progress,
                      QTextEdit *editor,
                      US_Hydrodyn *us_hydrodyn);

#define US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC   -1
#define US_HYDRODYN_SUPC_FILE_NOT_FOUND     -5
#define US_HYDRODYN_SUPC_OVERLAPS_EXIST     -6
#define US_HYDRODYN_SUPC_NO_SEL_MODELS      -7

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

class supc_thr_t : public QThread
{
 public:
  supc_thr_t(int);
  void supc_thr_setup(
                      unsigned int threads,
                      int mode,
                      float *p,
                      float *rRis,
                      float *rRi,
                      float *a,
                      int N,
                      int i,
                      int nat,
                      float *sum
                      );
  void supc_thr_shutdown();
  void supc_thr_wait();
  int supc_thr_work_status();
  virtual void run();

 private:

  unsigned int threads;

  int mode;

  float *p;
  float *rRis;
  float *rRi;
  float *a;
  int N;
  int i;
  int nat;
  float *sum;

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
