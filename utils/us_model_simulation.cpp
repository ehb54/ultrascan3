//! \file us_model_simulation.cpp
#include "us_model_simulation.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_lamm_astfvm.h"
#include "us_math2.h"
#include "us_settings.h"
#include "us_thread_worker.h"

// A thread gets more components than this, or the simulation runs in one thread
#define MIN_NTC 25

US_ModelSimulation::US_ModelSimulation( QObject* parent ) : QObject( parent )
{
}

QStringList US_ModelSimulation::prepare( IUS_DB2*                                              db,
                                         int                                                   invID,
                                         US_DataIO::EditedData&                                edata,
                                         const US_Model&                                       model,
                                         const QMap<QString, QString>&                         adv_vals,
                                         bool                                                  exp_steps,
                                         const QVector<US_SimulationParameters::SpeedProfile>& speed_steps,
                                         const QString&                                        tmst_fpath,
                                         US_SimulationParameters&                              simparams,
                                         US_DataIO::RawData&                                   sdata )
{
   int    nconc = edata.pointCount();
   double radlo = edata.radius( 0 );
   double radhi = edata.radius( nconc - 1 );

   // Initialize simulation parameters using edited data information
   simparams.initFromData( db, edata, !exp_steps, invID );
   simparams.simpoints         = adv_vals.value( "simpoints" ).toInt();
   simparams.meshType          = US_SimulationParameters::ASTFEM;
   simparams.gridType          = US_SimulationParameters::MOVING;
   simparams.radial_resolution = (double) ( radhi - radlo ) / ( nconc - 1 );

   if ( exp_steps )
   {
      simparams.speed_step = speed_steps;
   }

   QString gtyp = adv_vals.value( "gridtype" );
   QString bvol = adv_vals.value( "bandvolume" );

   if ( gtyp.contains( "Constant" ) )
   {
      simparams.gridType = US_SimulationParameters::FIXED;
   }

   if ( model.components[0].sigma == 0.0 && model.components[0].delta == 0.0 )
   {
      simparams.meshType = US_SimulationParameters::ASTFEM;
   }
   else
   {
      simparams.meshType = US_SimulationParameters::ASTFVM;
   }

   simparams.firstScanIsConcentration = false;

   double concval1 = 0.0;

   if ( simparams.band_forming )
   {
      simparams.band_volume = bvol.toDouble();
   }
   else
   {
      simparams.band_volume = 0.0;
   }

   // A simulation copy of the experimental data without readings
   US_AstfemMath::initSimData( sdata, edata, concval1 );

   QFileInfo check_file( tmst_fpath );
   simparams.sim = ( edata.channel == "S" );

   if ( ( check_file.exists() ) && ( check_file.isFile() ) )
   {
      if ( US_AstfemMath::timestate_onesec( tmst_fpath, sdata ) )
      { // Load timestate that is already at 1-second-intervals
         simparams.simSpeedsFromTimeState( tmst_fpath );
         simparams.speedstepsFromSSprof();
      }

      else
      {
         // Build the profile from this simulation's speed steps without changing the run files.
         simparams.tsobj = nullptr;
         if ( US_AstfemMath::timestate_from_speedsteps( simparams, sdata ) > 0 )
         {
            simparams.speedstepsFromSSprof();
         }
      }
   }
   else
   {
      if ( !simparams.sim )
      { // Compute acceleration rate for non-astfem_sim data
         const double tfac                    = ( 4.0 / 3.0 );
         double       t2                      = simparams.speed_step[0].time_first;
         double       w2t                     = simparams.speed_step[0].w2t_first;
         double       om1t                    = simparams.speed_step[0].rotorspeed * M_PI / 30.0;
         double       w2                      = sq( om1t );
         double       t1                      = tfac * ( t2 - ( w2t / w2 ) );
         int          t_acc                   = (int) qRound( t1 );
         double       rate                    = (double) ( simparams.speed_step[0].rotorspeed ) / (double) t_acc;
         simparams.speed_step[0].acceleration = (int) qRound( rate );
      }
   }

   return US_AstfemMath::check_acceleration( simparams.speed_step, edata.scanData );
}

int US_ModelSimulation::threadCount( int ncomp )
{
   int nthread = US_Settings::threads();
   int ntc     = ( ncomp + nthread - 1 ) / nthread;
   return ( ntc > MIN_NTC ) ? nthread : 1;
}

void US_ModelSimulation::simulate( US_Model&                model,
                                   US_SimulationParameters& simparams,
                                   US_DataIO::RawData&      sdata,
                                   US_Buffer&               buffer,
                                   double                   compress,
                                   bool                     manual,
                                   int                      dbg_level )
{
   if ( model.components[0].sigma == 0.0 && model.components[0].delta == 0.0 && model.coSedSolute < 0.0 && compress == 0.0 )
   {
      US_Astfem_RSA astfem_rsa( model, simparams );

      connect( &astfem_rsa, &US_Astfem_RSA::current_component, this, &US_ModelSimulation::component );
      astfem_rsa.set_debug_flag( dbg_level );
      buffer.compressibility = compress;
      buffer.manual          = manual;

      astfem_rsa.calculate( sdata );
   }
   else
   {
      // As the threaded path does: the buffer gives a co-sedimenting model its salt gradient
      buffer.compressibility = compress;
      buffer.manual          = manual;
      US_LammAstfvm astfvm( model, simparams );
      astfvm.set_buffer( buffer );
      astfvm.calculate( sdata );
   }
}

void US_ModelSimulation::start( US_Model&                model,
                                US_SimulationParameters& simparams,
                                US_DataIO::RawData&      a_sdata,
                                US_Buffer&               buffer,
                                double                   compress,
                                bool                     manual,
                                int                      a_nthread )
{
   int ncomp              = model.components.size();
   sdata                  = &a_sdata;
   nthread                = a_nthread;
   buffer.compressibility = compress;
   buffer.manual          = manual;
   tsimdats.clear();
   tmodels.clear();
   kcomps.clear();

   // Build models for each thread
   for ( int ii = 0; ii < ncomp; ii++ )
   {
      if ( ii < nthread )
      { // First time through per thread:  get initial model and sim data
         tmodels << model;
         tmodels[ii].components.clear();
         US_DataIO::RawData sdat = *sdata;
         tsimdats << sdat;
         kcomps << 0;
      }

      // Partition thread models from round-robin fetch of components
      int jj = ii % nthread;
      tmodels[jj].components << model.components[ii];
   }

   thrdone       = 0;
   buffer.manual = manual;

   // Build worker threads and begin running
   for ( int ii = 0; ii < nthread; ii++ )
   {
      US_ThreadWorker* tworker = new US_ThreadWorker( tmodels[ii], simparams, tsimdats[ii], buffer, ii );
      US_ThreadEx*     wthread = new US_ThreadEx();

      tworker->moveToThread( wthread );

      connect( wthread, &QThread::started, tworker, &US_ThreadWorker::calc_simulation );

      connect( tworker, &US_ThreadWorker::work_progress, this, &US_ModelSimulation::thread_progress );
      connect( tworker, &US_ThreadWorker::work_complete, this, &US_ModelSimulation::thread_complete );
      // Once its part is done, the thread stops; then it and its worker are deleted
      connect( tworker, &US_ThreadWorker::work_complete, wthread, &QThread::quit );
      connect( wthread, &QThread::finished, tworker, &QObject::deleteLater );
      connect( wthread, &QThread::finished, wthread, &QObject::deleteLater );

      wthread->start();
   }
}

// Update the count of components done in all threads
void US_ModelSimulation::thread_progress( int thr, int icomp )
{
   int kcomp   = 0;
   kcomps[thr] = icomp;
   for ( int ii = 0; ii < nthread; ii++ )
   {
      kcomp += kcomps[ii];
   }
   emit progress( kcomp );
}

// Count the threads done; when all are, add their simulations into sdata
void US_ModelSimulation::thread_complete( int )
{
   thrdone++;

   if ( thrdone >= nthread )
   {
      for ( int ii = 0; ii < sdata->scanData.size(); ii++ )
      {
         for ( int jj = 0; jj < sdata->xvalues.size(); jj++ )
         {
            double conc = sdata->value( ii, jj );

            for ( int kk = 0; kk < nthread; kk++ )
            {
               conc += tsimdats[kk].value( ii, jj );
            }

            sdata->setValue( ii, jj, conc );
         }
      }

      emit finished();
   }
}
