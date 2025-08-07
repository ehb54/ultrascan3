//! \file us_worker_calcnorm.cpp
#include "us_worker_calcnorm.h"

#include "us_astfem_math.h"
#include "us_constants.h"
#include "us_memory.h"
#include "us_model.h"
#include "us_settings.h"
#include "us_sleep.h"
#include "us_util.h"

// construct worker thread to calculate norms
WorkerThreadCalcNorm::WorkerThreadCalcNorm(QObject* parent) : QThread(parent) {
  dset = NULL;
  thrn = -1;
  attr_x = 0;
  attr_y = 1;
  attr_z = 3;
  amask = 0;
  cff0 = 0.0;
  dbg_level = US_Settings::us_debug();
  DbgLv(1) << "CN(WT): Thread created";
}

// worker thread destructor
WorkerThreadCalcNorm::~WorkerThreadCalcNorm() {
  DbgLv(1) << "CN(WT):   Thread destroy - (1)finished?" << isFinished() << thrn;
  if (!wait(2000)) {
    qDebug() << "Thread destroy wait timeout(2secs) : Thread" << thrn;
  }
  DbgLv(1) << "CN(WT):   Thread destroy - (2)finished?" << isFinished() << thrn;
  DbgLv(1) << "CN(WT):    Thread destroyed" << thrn;
}

// define work for a worker thread
void WorkerThreadCalcNorm::define_work(WorkPacketCN& workin) {
  thrn = workin.thrn;     // thread number
  nthrd = workin.nthrd;   // total threads count
  dset = workin.dset;     // dataset pointer
  amask = workin.amask;   // xyz attribute mask
  cff0 = workin.cff0;     // constant f/f0 value
  attr_x = (amask >> 6);  // attribute indecies
  attr_y = (amask >> 3) & 7;
  attr_z = amask & 7;
  DbgLv(1) << "define_work_threadno." << thrn << " of" << nthrd << attr_x
           << attr_y << attr_z;
  solutes_i = workin.isolutes;   // full solute points vector
  nsolutes = solutes_i.count();  // total solute points
}

// get results of a completed worker thread
void WorkerThreadCalcNorm::get_result(WorkPacketCN& workout) {
  workout.thrn = thrn;
  workout.nthrd = nthrd;
  workout.dset = dset;
  workout.amask = amask;
  workout.csolutes = solutes_c;
  workout.solxs = solxs;
  workout.nsolutes = nsolutes;
  workout.nwsols = nwsols;
  //*DEBUG*
  // int nn=workout.csolutes.size();
  // int kk=nn/2;
  // int ni=solutes_i.size();
  // DbgLv(1) << "CN(WT): thr nn" << thrn << nn << "out sol0 solk soln"
  // << workout.csolutes[0].c << workout.csolutes[kk].c <<
  // workout.csolutes[nn-1].c
  // << "in sol0 soln" << ni << solutes_i[0].s*1.e13 << solutes_i[ni-1].s*1.e13
  // << solutes_i[0].c << solutes_i[ni-1].c;
  //*DEBUG*
  DbgLv(1) << "get_result" << workout.csolutes.size() << workout.nthrd;
}

// run the worker thread
void WorkerThreadCalcNorm::run() {
  DbgLv(1) << "CN(WT):  run: calc_norms:";
  calc_norms();  // do all the work here
  quit();
  exec();
}

// Do the real work of a thread:  norm values for each of its solutes
void WorkerThreadCalcNorm::calc_norms() {
  DbgLv(1) << "calc_norms is called" << nsolutes << nthrd;

  for (int ii = (thrn - 1); ii < nsolutes;
       ii += nthrd) {  // fill list with this worker's solute point indecies
    solxs << ii;
    DbgLv(1) << "solxs_values" << solxs;
  }

  nwsols = solxs.count();  // count of solutes for worker
  DbgLv(1) << "nwsols_" << nwsols << "solx0" << solxs[0] << "solx1" << solxs[1]
           << "solxn" << solxs[nwsols - 1];

  solutes_c.resize(nwsols);  // computed solute points

  for (int ii = 0; ii < nwsols;
       ii++) {  // computed solutes initialized from full input list
    solutes_c[ii] = solutes_i[solxs[ii]];
    solutes_c[ii].c = 0.0;
  }
  DbgLv(1) << "CN(WT):  CN:  sol_c0.s" << solutes_c[0].s;

  simparms = dset->simparams;  // local simulation parameters

  US_DataIO::RawData simdat;  // simulation data set
  US_Model model1;            // 1-component work model
  model1.components.resize(1);

  // Initialize the simulation data set
  US_AstfemMath::initSimData(simdat, dset->run_data, 0.0);

  int nscan = simdat.scanCount();
  int npoint = simdat.pointCount();
  DbgLv(1) << "CN(WT):  CN:  nscan" << nscan << "npoint" << npoint;

  // Zeroed model component for initialization
  US_Model::SimulationComponent zcomponent;
  zcomponent.s = 0.0;
  zcomponent.D = 0.0;
  zcomponent.mw = 0.0;
  zcomponent.f = 0.0;
  zcomponent.f_f0 = cff0;
  zcomponent.vbar20 = dset->vbar20;

  // Do the work of finite element modeling, norm value from simulation
  for (int ii = 0; ii < nwsols; ii++) {
    // Initialize component, then set X,Y,Z from solute point
    model1.components[0] = zcomponent;
    set_comp_attr(model1.components[0], solutes_c[ii], attr_x);
    set_comp_attr(model1.components[0], solutes_c[ii], attr_y);
    set_comp_attr(model1.components[0], solutes_c[ii], attr_z);

    // Compute the other coefficients
    model1.update_coefficients();

    // Convert to 20w space
    model1.components[0].s /= dset->s20w_correction;
    model1.components[0].D /= dset->D20w_correction;

    // Reinitialize the simulation data set initial concentrations
    for (int jj = 0; jj < nscan; jj++)
      for (int kk = 0; kk < npoint; kk++) simdat.setValue(jj, kk, 0.0);

    DbgLv(1) << "CN(WT):  CN:   ii" << ii << "astfem_rsa:";
    // Perform finite element modeling to compute the simulation
    US_Astfem_RSA astfem_rsa(model1, simparms);

    astfem_rsa.calculate(simdat);

    // Store the norm value for this simulation (A matrix column)
    double znorm = US_Math2::norm_value(&simdat);
    solutes_c[ii].c = znorm;
    // Signal a completed step (solute point)
    emit work_progress(1);
  }

  // Signal that a thread's work is done
  emit work_complete(this);
}

// Set a model component coefficient from a solute attribute
void WorkerThreadCalcNorm::set_comp_attr(
    US_Model::SimulationComponent& component, US_Solute& solute,
    int attr_type) {
  switch (attr_type) {
    default:
    case ATTR_S:  // Sedimentation Coefficient
      component.s = solute.s;
      break;
    case ATTR_K:  // Frictional Ratio
      component.f_f0 = solute.k;
      break;
    case ATTR_W:  // Molecular Weight
      component.mw = solute.d;
      break;
    case ATTR_V:  // Partial Specific Volume (vbar)
      component.vbar20 = solute.v;
      break;
    case ATTR_D:  // Diffusion Coefficient
      component.D = solute.d;
      break;
    case ATTR_F:  // Frictional Coefficient
      component.f = solute.d;
      break;
  }
}
