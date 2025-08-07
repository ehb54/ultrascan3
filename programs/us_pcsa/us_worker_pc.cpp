//! \file us_worker_pc.cpp
#include "us_worker_pc.h"

#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_memory.h"
#include "us_model.h"
#include "us_settings.h"
#include "us_sleep.h"
#include "us_util.h"

// construct worker thread
WorkerThreadPc::WorkerThreadPc(QObject* parent) : QThread(parent) {
  dbg_level = US_Settings::us_debug();
  abort = false;
  solvesim = NULL;
  thrn = -1;
  depth = 0;
  DbgLv(1) << "PC(WT): Thread created";
}

// worker thread destructor
WorkerThreadPc::~WorkerThreadPc() {
#if 1
  if (solvesim != NULL) delete solvesim;
#endif

  DbgLv(1) << "PC(WT):   Thread destroy - (1)finished?" << isFinished() << thrn;
  if (!wait(2000)) {
    DbgLv(1) << "Thread destroy wait timeout(2secs) : Thread" << thrn;
  }
  DbgLv(1) << "PC(WT):   Thread destroy - (2)finished?" << isFinished() << thrn;
  DbgLv(1) << "PC(WT):    Thread destroyed" << thrn;
}

// define work for a worker thread
void WorkerThreadPc::define_work(WorkPacketPc& workin) {
  mutex.lock();
  str_y = workin.str_y;
  end_y = workin.end_y;
  par1 = workin.par1;
  par2 = workin.par2;
  par3 = workin.par3;
  thrn = workin.thrn;
  taskx = workin.taskx;
  noisflag = workin.noisf;
  depth = workin.depth;
  psv_nnls_a = workin.psv_nnls_a;
  psv_nnls_b = workin.psv_nnls_b;

  solutes_i = workin.isolutes;

  QString phdr = QString("PC(WT)dw:%1:%2:").arg(taskx).arg(thrn);
  if (depth > 0) {
    DbgLv(1) << phdr << "depth1 psv_nnlsab" << psv_nnls_a << psv_nnls_b;
  } else {
    DbgLv(1) << phdr << "DefWk: sols size" << solutes_i.size();
  }
  dset_wk = *(workin.dsets[0]);  // local copy of data set
  dset_wk.noise_files = workin.dsets[0]->noise_files;
  dset_wk.run_data = workin.dsets[0]->run_data;
  dset_wk.model = workin.dsets[0]->model;
  dset_wk.simparams = workin.dsets[0]->simparams;
  dset_wk.solution_rec = workin.dsets[0]->solution_rec;
  dset_wk.solute_type = workin.dsets[0]->solute_type;
  dsets.clear();
  dsets << &dset_wk;  // save its pointer
  DbgLv(1) << phdr << "DefWk:   stype" << dsets[0]->solute_type;

  sim_vals = workin.sim_vals;
  sim_vals.variance = workin.sim_vals.variance;
  sim_vals.ti_noise = workin.sim_vals.ti_noise;
  sim_vals.ri_noise = workin.sim_vals.ti_noise;
  sim_vals.zsolutes = solutes_i;
  mutex.unlock();
}

// get results of a completed worker thread
void WorkerThreadPc::get_result(WorkPacketPc& workout) {
  mutex.lock();
  DbgLv(1) << "PC(WT): get_result IN";
  workout.str_y = str_y;
  workout.end_y = end_y;
  workout.par1 = par1;
  workout.par2 = par2;
  workout.par3 = par3;
  workout.thrn = thrn;
  workout.taskx = taskx;
  workout.noisf = noisflag;
  workout.depth = depth;

  workout.isolutes = solutes_i;
  workout.csolutes = solutes_c;
  workout.ti_noise = ti_noise.values;
  workout.ri_noise = ri_noise.values;
  workout.sim_vals = sim_vals;
  workout.dsets = dsets;
  //*DEBUG*
  int nn = workout.csolutes.size();
  int kk = nn / 2;
  int ni = solutes_i.size();
  if (depth == 0) {
    DbgLv(1) << "PC(WT): thr nn" << thrn << nn << "out sol0 solk soln"
             << workout.csolutes[0].c << workout.csolutes[kk].c
             << workout.csolutes[nn - 1].c << "ni sol0 soln x" << ni
             << solutes_i[0].x * 1.e13 << solutes_i[ni - 1].x * 1.e13 << "c"
             << solutes_i[0].c << solutes_i[ni - 1].c;
  } else {
    DbgLv(1) << "PC(WT): thr nn" << thrn << nn << "ni sol0 soln x" << ni
             << solutes_i[0].x * 1.e13 << solutes_i[ni - 1].x * 1.e13 << "c"
             << solutes_i[0].c << solutes_i[ni - 1].c;
  }
  //*DEBUG*
  mutex.unlock();
}

// run the worker thread
void WorkerThreadPc::run() {
  QString phdr = QString("WT:RUN:%1:%2:").arg(taskx).arg(thrn);
  calc_residuals();  // do all the work here

  DbgLv(1) << phdr << "   sig w_c";
  emit work_complete(this);  // signal that a thread's work is done

  DbgLv(1) << phdr << "     c_r return";
  quit();
  exec();
}

// set a flag so that a worker thread will abort as soon as possible
void WorkerThreadPc::flag_abort() { solvesim->abort_work(); }

// Do the real work of a thread:  solution from solutes set
void WorkerThreadPc::calc_residuals() {
  QString phdr = QString("PC(WT):CR:%1:%2:").arg(taskx).arg(thrn);
  DbgLv(1) << phdr << "depth" << depth;

  if (depth == 0) {  // Fit task:  do full compute of model
    solvesim = new US_SolveSim(dsets, thrn, true);
    DbgLv(1) << phdr << " A)dsets size" << dsets.size();

    sim_vals.zsolutes = solutes_i;
    sim_vals.noisflag = noisflag;
    sim_vals.dbg_level = dbg_level;
    sim_vals.dbg_timing = US_Settings::debug_match("pcsaTiming");
    int ns = solutes_i.size();
    DbgLv(1) << phdr << " B)sols_i size" << ns << "stype"
             << dsets[0]->solute_type;
    for (int js = 0; js < ns; js++) {
      if (js < 4 || (js + 5) > ns)
        DbgLv(1) << phdr << "  soli: js" << js << " sol.x,y,z,c"
                 << solutes_i[js].x << solutes_i[js].y << solutes_i[js].z
                 << solutes_i[js].c;
    }

    solvesim->calc_residuals(0, 1, sim_vals);

    solutes_c = sim_vals.zsolutes;
    DbgLv(1) << phdr << " C)sols_c size" << solutes_c.size();
    ti_noise.values = sim_vals.ti_noise;
    ri_noise.values = sim_vals.ri_noise;
    DbgLv(1) << phdr << "sim,res ptCounts" << sim_vals.sim_data.pointCount()
             << sim_vals.residuals.pointCount();
  }

  else {  // Alpha scan task:  apply alpha using saved A,B matrices
    int nscans = dsets[0]->run_data.scanCount();
    int npoints = dsets[0]->run_data.pointCount();
    int nisols = solutes_i.size();
    double variance = 0.0;
    double xnormsq = 0.0;
    double alpha = sim_vals.alpha;

    DbgLv(1) << phdr << "   call apply_alpha" << alpha;
    apply_alpha(alpha, psv_nnls_a, psv_nnls_b, nscans, npoints, nisols,
                variance, xnormsq);
    DbgLv(1) << phdr << "     get apply_alpha: vari xnsq" << variance
             << xnormsq;

    sim_vals.variance = variance;
    sim_vals.xnormsq = xnormsq;
    DbgLv(1) << phdr << "      sv.xnormsq" << sim_vals.xnormsq;
  }

  return;
}

// Slot to forward a progress signal
void WorkerThreadPc::forward_progress(int steps) { emit work_progress(steps); }

void WorkerThreadPc::apply_alpha(const double alpha,
                                 QVector<double>* psv_nnls_a,
                                 QVector<double>* psv_nnls_b, const int nscans,
                                 const int npoints, const int nisols,
                                 double& variance, double& xnormsq) {
  QString phdr = QString("wAA:%1:%2:").arg(taskx).arg(thrn);
  int ntotal = nscans * npoints;
  int narows = ntotal + nisols;
  int navals = narows * nisols;
  int ncsols = 0;
  variance = 0.0;
  xnormsq = 0.0;
  QVector<double> nnls_a(navals);  // Local copy of A matrix
  QVector<double> nnls_b(narows);  // Local copy of b matrix
  QVector<double> nnls_x(nisols);  // Local copy of x matrix
  QVector<double> simdat;          // Simulation vector

  // Fill local copies of matrices
  mutex.lock();

  for (int jj = 0; jj < navals; jj++) nnls_a[jj] = psv_nnls_a->at(jj);

  for (int jj = 0; jj < narows; jj++) nnls_b[jj] = psv_nnls_b->at(jj);

  nnls_x.fill(0.0, nisols);

  int kavl = psv_nnls_a->size();
  mutex.unlock();
  DbgLv(1) << phdr << " ns np ni na" << nscans << npoints << nisols << narows;
  // for(int jj=0;jj<(narows*2);jj++)
  //{ nnls_a << 0.0; nnls_b << 0.0; nnls_x << 0.0; }

  // Replace alpha in the diagonal of the lower square of A
  double* a_ptr = nnls_a.data();
  double* b_ptr = nnls_b.data();
  double* x_ptr = nnls_x.data();
  int dx = ntotal;
  int dinc = narows + 1;
  DbgLv(1) << phdr << " alpha" << alpha << "dx dinc" << dx << dinc;

  for (int cc = 0; cc < nisols; cc++, dx += dinc) {
    a_ptr[dx] = alpha;
  }

  // Compute the X vector using NNLS
  DbgLv(1) << phdr << "pre-nnls";
  US_Math2::nnls(a_ptr, narows, narows, nisols, b_ptr, x_ptr);

  DbgLv(1) << phdr << "post-nnls  rss_now" << US_Memory::rss_now();
  nnls_a.clear();  // Free work A and b matrices
  nnls_b.clear();
  simdat.fill(0.0, ntotal);  // Initialize simulation vector
  int ktot = simdat.size();
  DbgLv(1) << phdr << "  simdat fill ktot,kavl,naro" << ktot << kavl << narows
           << "rss_now" << US_Memory::rss_now();
  double* s_ptr = simdat.data();
  a_ptr = psv_nnls_a->data();
  b_ptr = psv_nnls_b->data();

  // Construct the output solutes and the implied simulation and xnorm-sq
  for (int cc = 0; cc < nisols; cc++) {
    double soluval = x_ptr[cc];  // Computed concentration, this solute

    if (soluval > 0.0) {
      xnormsq += sq(soluval);
      int aa = cc * narows;

      for (int kk = 0; kk < ntotal; kk++) {
        s_ptr[kk] += (soluval * a_ptr[aa++]);
      }

      ncsols++;
    }
  }

  // Calculate the sum for the variance computation
  for (int kk = 0; kk < ntotal; kk++) {
    variance += sq((b_ptr[kk] - s_ptr[kk]));
  }
  DbgLv(1) << phdr << "    ntot ncsols" << ntotal << ncsols << "varisum"
           << variance;

  // Return computed variance and xnorm-sq
  variance /= (double)ntotal;
  DbgLv(1) << phdr << " alpha" << alpha << "vari xnsq" << variance << xnormsq;
  int mm = npoints / 2;
  DbgLv(1) << phdr << " mm=" << mm << "a[m] b[m] s[m]" << (*psv_nnls_a)[mm]
           << (*psv_nnls_b)[mm] << simdat[mm] << "rss_now"
           << US_Memory::rss_now();
}
