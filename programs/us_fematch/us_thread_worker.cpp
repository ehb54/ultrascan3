//! \file us_thread_worker.cpp
#include "us_thread_worker.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_lamm_astfvm.h"
#include "us_model.h"
#include "us_sleep.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_memory.h"


// Construct worker thread
ThreadWorker::ThreadWorker( US_Model& a_model, US_SimulationParameters& params,
    US_DataIO::RawData& simda, US_Buffer& a_buff, int thr )
   : QObject(), model( a_model ), simparams( params ),
   simdat( simda ), buffer( a_buff )
{
   thrn         = thr;
   dbg_level    = US_Settings::us_debug();
DbgLv(1) << "THRWRK: Thread created" << thrn;
}


// Do the real work of a thread:  simulation solution from model
void ThreadWorker::calc_simulation()
{
DbgLv(1) << "THRWRK:CLCRES threadid" << QThread::currentThreadId();
DbgLv(1) << " THRWRK:" << thrn << "simdat scans radii"
   << simdat.scanData.size() << simdat.xvalues.size();
DbgLv(1) << " THRWRK:" << thrn << "model components"
   << model.components.size();
DbgLv(1) << " THRWRK:" << thrn << "model-0 s k" << model.components[0].s*1.e13
   << model.components[0].f_f0;
   double compress    = buffer.compressibility;

   if ( simparams.meshType != US_SimulationParameters::ASTFVM )
   {
      US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
   
      connect( astfem_rsa, SIGNAL( current_component( int ) ),
               this,       SLOT(   forward_progress ( int ) ) );

qint64 stim=QDateTime::currentDateTime().toMSecsSinceEpoch();
DbgLv(1) << " THRWRK:" << thrn << "calc START" << stim;
      astfem_rsa->calculate( simdat );
qint64 etim=QDateTime::currentDateTime().toMSecsSinceEpoch();
DbgLv(1) << " THRWRK:" << thrn << "calc    END" << etim;
   }

   else
   {
      US_LammAstfvm *astfvm     = new US_LammAstfvm( model, simparams );

      connect( astfvm,     SIGNAL( comp_progress   ( int ) ),
               this,       SLOT  ( forward_progress( int ) ) );

      astfvm->set_buffer( buffer );
      astfvm->calculate( simdat );
   }

   emit work_complete( thrn );
   qApp->processEvents();
   return;
}

// Slot to forward a progress signal
void ThreadWorker::forward_progress( int steps )
{
   emit work_progress( thrn, steps );
   qApp->processEvents();
qint64 etim=QDateTime::currentDateTime().toMSecsSinceEpoch();
DbgLv(1) << " THRWRK:" << thrn << "  progress TM" << etim;
}

