//! \file test_us_model_simulation.cpp
// US_ModelSimulation against the code it replaced in FEMatch, autoflow analysis
// and the GMP reporter, kept here word for word as it was at 94e74d4f3.
#include "qt_test_base.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_lamm_astfvm.h"
#include "us_math2.h"
#include "us_model.h"
#include "us_model_simulation.h"
#include "us_settings.h"
#include "us_simparms.h"
#include "us_thread_worker.h"
#include "us_time_state.h"
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <cmath>
#include <cstring>
#include <memory>
#include <random>
#ifdef Q_OS_MACOS
#include <mach/mach.h>
#endif

namespace
{
// The old code logs through qDebug and DbgLv; keep the test output quiet.
#undef qDebug
#define qDebug QT_NO_QDEBUG_MACRO
#ifndef DbgLv
#define DbgLv( a )                                                                                                                 \
   if ( dbg_level >= a )                                                                                                           \
   qDebug()
#endif

// The members and locals the old set-up code uses, with the programs' names
struct FrozenSetup
{
      US_DataIO::EditedData*                         edata = nullptr;
      US_DataIO::RawData*                            sdata = nullptr;
      US_SimulationParameters                        simparams;
      QMap<QString, QString>                         adv_vals;
      bool                                           exp_steps = false;
      bool                                           dat_steps = true;
      QVector<US_SimulationParameters::SpeedProfile> speed_steps;
      US_Model                                       model;
      int                                            dbg_level = 0;
};

struct FrozenFeMatchSetup : FrozenSetup
{
      US_DB2*     dbP = nullptr;
      QString     runID;
      int         drow = 0;
      QStringList run()
      {
         int    nconc = edata->pointCount();
         double radlo = edata->radius( 0 );
         double radhi = edata->radius( nconc - 1 );
         simparams.initFromData( dbP, *edata, dat_steps );
         simparams.simpoints         = adv_vals["simpoints"].toInt();
         simparams.meshType          = US_SimulationParameters::ASTFEM;
         simparams.gridType          = US_SimulationParameters::MOVING;
         simparams.radial_resolution = (double) ( radhi - radlo ) / ( nconc - 1 );
         //   simparams.bottom            = simparams.bottom_position;
         DbgLv( 1 ) << "SimMdl: simpoints" << simparams.simpoints << "rreso" << simparams.radial_resolution << "bottom_sim"
                    << simparams.bottom << "bottom_dat" << edata->bottom;
         //simparams.meniscus          = 5.8;

         //sdata.scanData.resize( total_scans );
         //int terpsize    = ( points + 7 ) / 8;

         if ( exp_steps )
         {
            simparams.speed_step = speed_steps;
         }

         DbgLv( 1 ) << "SimMdl: speed_steps:" << simparams.speed_step.size();

         QString mtyp = adv_vals["meshtype"];
         QString gtyp = adv_vals["gridtype"];
         QString bvol = adv_vals["bandvolume"];

#if 0
      if ( mtyp.contains( "Claverie" ) )
         simparams.meshType = US_SimulationParameters::CLAVERIE;
      else if ( mtyp.contains( "Moving Hat" ) )
         simparams.meshType = US_SimulationParameters::MOVING_HAT;
      else if ( mtyp.contains( "File:"      ) )
         simparams.meshType = US_SimulationParameters::USER;
      else if ( mtyp.contains( "ASTFVM"     ) )
      {
         simparams.meshType = US_SimulationParameters::ASTFVM;
         qDebug() << "meshtype= fvm";
      }
#endif
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
            //concval1              = 1.0;
            //simparams.firstScanIsConcentration = true;
         }
         else
         {
            simparams.band_volume = 0.0;
         }

         // Make a simulation copy of the experimental data without actual readings

         US_AstfemMath::initSimData( *sdata, *edata, concval1 ); // Gets experimental time grid set

         QString   tmst_fpath = US_Settings::resultDir() + "/" + runID + "/" + runID + ".time_state.tmst";
         QFileInfo check_file( tmst_fpath );
         simparams.sim = ( edata->channel == "S" );

         if ( ( check_file.exists() ) && ( check_file.isFile() ) )
         {
            if ( US_AstfemMath::timestate_onesec( tmst_fpath, *sdata ) )
            { // Load timestate that is already at 1-second-intervals
               simparams.simSpeedsFromTimeState( tmst_fpath );
               DbgLv( 1 ) << "SimMdl: timestate file exists" << tmst_fpath << " timestateobject,count = " << simparams.tsobj
                          << simparams.sim_speed_prof.count();
               simparams.speedstepsFromSSprof();
            }

            else
            { // Replace timestate with a new one that is at 1-second-intervals
               if ( drow == 0 )
               {
                  QString tmst_fdefs = QString( tmst_fpath ).replace( ".tmst", ".xml" );
                  QString tmst_fpsav = tmst_fpath + "-orig";
                  QString tmst_fdsav = tmst_fdefs + "-orig";
                  simparams.sim      = ( edata->channel == "S" );

                  // Rename existing (non-1sec-intv) files
                  QFile::rename( tmst_fpath, tmst_fpsav );
                  QFile::rename( tmst_fdefs, tmst_fdsav );
                  // Create a new 1-second-interval file set
                  US_AstfemMath::writetimestate( tmst_fpath, simparams, *sdata );
                  DbgLv( 1 ) << "SimMdl: 1-sec-intv file created";
               }

               simparams.simSpeedsFromTimeState( tmst_fpath );
               simparams.speedstepsFromSSprof();
            }
         }
         else
         {
            DbgLv( 1 ) << "SimMdl: timestate file does not exist";
            if ( !simparams.sim )
            { // Compute acceleration rate for non-astfem_sim data
               const double tfac  = ( 4.0 / 3.0 );
               double       t2    = simparams.speed_step[0].time_first;
               double       w2t   = simparams.speed_step[0].w2t_first;
               double       om1t  = simparams.speed_step[0].rotorspeed * M_PI / 30.0;
               double       w2    = sq( om1t );
               double       t1    = tfac * ( t2 - ( w2t / w2 ) );
               int          t_acc = (int) qRound( t1 );
               double       rate  = (double) ( simparams.speed_step[0].rotorspeed ) / (double) t_acc;
               DbgLv( 1 ) << "SimMdl:  accel-calc:  t1 t2 w2t t_acc speed rate" << t1 << t2 << w2t << t_acc
                          << simparams.speed_step[0].rotorspeed << rate;
               simparams.speed_step[0].acceleration = (int) qRound( rate );
            }
         }
         QStringList check_results = US_AstfemMath::check_acceleration( simparams.speed_step, edata->scanData );
         return check_results;
      }
};

struct FrozenAutoflowSetup : FrozenSetup
{
      US_DB2*     dbP = nullptr;
      QString     FileName_parsed;
      QStringList run()
      {
         int    nconc = edata->pointCount();
         double radlo = edata->radius( 0 );
         double radhi = edata->radius( nconc - 1 );
         exp_steps    = US_SimulationParameters::speedStepsCoverData( speed_steps, *edata );
         dat_steps    = !exp_steps;
         simparams.initFromData( dbP, *edata, dat_steps );
         simparams.simpoints         = adv_vals["simpoints"].toInt();
         simparams.meshType          = US_SimulationParameters::ASTFEM;
         simparams.gridType          = US_SimulationParameters::MOVING;
         simparams.radial_resolution = (double) ( radhi - radlo ) / ( nconc - 1 );
         //   simparams.bottom            = simparams.bottom_position;
         qDebug() << "SimMdl: simpoints" << simparams.simpoints << "rreso" << simparams.radial_resolution << "bottom_sim"
                  << simparams.bottom << "bottom_dat" << edata->bottom;
         //simparams.meniscus          = 5.8;

         //sdata.scanData.resize( total_scans );
         //int terpsize    = ( points + 7 ) / 8;

         if ( exp_steps )
         {
            simparams.speed_step = speed_steps;
         }

         qDebug() << "SimMdl: speed_steps:" << simparams.speed_step.size();

         QString mtyp = adv_vals["meshtype"];
         QString gtyp = adv_vals["gridtype"];
         QString bvol = adv_vals["bandvolume"];


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
            //concval1              = 1.0;
            //simparams.firstScanIsConcentration = true;
         }
         else
         {
            simparams.band_volume = 0.0;
         }

         // Make a simulation copy of the experimental data without actual readings

         qDebug() << "SimulateModel: --- 1";

         US_AstfemMath::initSimData( *sdata, *edata, concval1 ); // Gets experimental time grid set

         qDebug() << "SimulateModel: --- 2";


         QString   tmst_fpath = US_Settings::resultDir() + "/" + FileName_parsed + "/" + FileName_parsed + ".time_state.tmst";
         QFileInfo check_file( tmst_fpath );
         simparams.sim = ( edata->channel == "S" );

         if ( ( check_file.exists() ) && ( check_file.isFile() ) )
         {
            if ( US_AstfemMath::timestate_onesec( tmst_fpath, *sdata ) )
            { // Load timestate that is already at 1-second-intervals
               simparams.simSpeedsFromTimeState( tmst_fpath );
               qDebug() << "SimMdl: timestate file exists" << tmst_fpath << " timestateobject,count = " << simparams.tsobj
                        << simparams.sim_speed_prof.count();
               simparams.speedstepsFromSSprof();
            }

            else
            { // Replace timestate with a new one that is at 1-second-intervals
               // if ( drow == 0 )
               //   {
               QString tmst_fdefs = QString( tmst_fpath ).replace( ".tmst", ".xml" );
               QString tmst_fpsav = tmst_fpath + "-orig";
               QString tmst_fdsav = tmst_fdefs + "-orig";
               simparams.sim      = ( edata->channel == "S" );

               // Rename existing (non-1sec-intv) files
               QFile::rename( tmst_fpath, tmst_fpsav );
               QFile::rename( tmst_fdefs, tmst_fdsav );
               // Create a new 1-second-interval file set
               US_AstfemMath::writetimestate( tmst_fpath, simparams, *sdata );
               qDebug() << "SimMdl: 1-sec-intv file created";
               // }

               simparams.simSpeedsFromTimeState( tmst_fpath );
               simparams.speedstepsFromSSprof();
            }
         }
         else
         {
            qDebug() << "SimMdl: timestate file does not exist";
            if ( !simparams.sim )
            { // Compute acceleration rate for non-astfem_sim data
               const double tfac  = ( 4.0 / 3.0 );
               double       t2    = simparams.speed_step[0].time_first;
               double       w2t   = simparams.speed_step[0].w2t_first;
               double       om1t  = simparams.speed_step[0].rotorspeed * M_PI / 30.0;
               double       w2    = sq( om1t );
               double       t1    = tfac * ( t2 - ( w2t / w2 ) );
               int          t_acc = (int) qRound( t1 );
               double       rate  = (double) ( simparams.speed_step[0].rotorspeed ) / (double) t_acc;
               qDebug() << "SimMdl:  accel-calc:  t1 t2 w2t t_acc speed rate" << t1 << t2 << w2t << t_acc
                        << simparams.speed_step[0].rotorspeed << rate;
               simparams.speed_step[0].acceleration = (int) qRound( rate );
            }
         }
         QStringList check_results = US_AstfemMath::check_acceleration( simparams.speed_step, edata->scanData );
         return check_results;
      }
};

struct FrozenReporterSetup : FrozenSetup
{
      std::unique_ptr<US_DB2> dbP;
      QString                 FileName_parsed;
      QStringList             run()
      {
         int    nconc = edata->pointCount();
         double radlo = edata->radius( 0 );
         double radhi = edata->radius( nconc - 1 );
         exp_steps    = US_SimulationParameters::speedStepsCoverData( speed_steps, *edata );
         dat_steps    = !exp_steps;
         simparams.initFromData( dbP.get(), *edata, dat_steps );
         simparams.simpoints         = adv_vals["simpoints"].toInt();
         simparams.meshType          = US_SimulationParameters::ASTFEM;
         simparams.gridType          = US_SimulationParameters::MOVING;
         simparams.radial_resolution = (double) ( radhi - radlo ) / ( nconc - 1 );
         //   simparams.bottom            = simparams.bottom_position;
         qDebug() << "SimMdl: simpoints" << simparams.simpoints << "rreso" << simparams.radial_resolution << "bottom_sim"
                  << simparams.bottom << "bottom_dat" << edata->bottom;
         //simparams.meniscus          = 5.8;

         //sdata.scanData.resize( total_scans );
         //int terpsize    = ( points + 7 ) / 8;


         //test
         dbP.reset();

         if ( exp_steps )
         {
            simparams.speed_step = speed_steps;
         }

         qDebug() << "SimMdl: speed_steps:" << simparams.speed_step.size();

         QString mtyp = adv_vals["meshtype"];
         QString gtyp = adv_vals["gridtype"];
         QString bvol = adv_vals["bandvolume"];


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
            //concval1              = 1.0;
            //simparams.firstScanIsConcentration = true;
         }
         else
         {
            simparams.band_volume = 0.0;
         }

         // Make a simulation copy of the experimental data without actual readings

         qDebug() << "SimulateModel: --- 1";

         US_AstfemMath::initSimData( *sdata, *edata, concval1 ); // Gets experimental time grid set

         qDebug() << "SimulateModel: --- 2";


         QString   tmst_fpath = US_Settings::resultDir() + "/" + FileName_parsed + "/" + FileName_parsed + ".time_state.tmst";
         QFileInfo check_file( tmst_fpath );
         simparams.sim = ( edata->channel == "S" );

         if ( ( check_file.exists() ) && ( check_file.isFile() ) )
         {
            if ( US_AstfemMath::timestate_onesec( tmst_fpath, *sdata ) )
            { // Load timestate that is already at 1-second-intervals
               simparams.simSpeedsFromTimeState( tmst_fpath );
               qDebug() << "SimMdl: timestate file exists" << tmst_fpath << " timestateobject,count = " << simparams.tsobj
                        << simparams.sim_speed_prof.count();
               simparams.speedstepsFromSSprof();
            }

            else
            { // Replace timestate with a new one that is at 1-second-intervals
               // if ( drow == 0 )
               //   {
               QString tmst_fdefs = QString( tmst_fpath ).replace( ".tmst", ".xml" );
               QString tmst_fpsav = tmst_fpath + "-orig";
               QString tmst_fdsav = tmst_fdefs + "-orig";
               simparams.sim      = ( edata->channel == "S" );

               // Rename existing (non-1sec-intv) files
               QFile::rename( tmst_fpath, tmst_fpsav );
               QFile::rename( tmst_fdefs, tmst_fdsav );
               // Create a new 1-second-interval file set
               US_AstfemMath::writetimestate( tmst_fpath, simparams, *sdata );
               qDebug() << "SimMdl: 1-sec-intv file created";
               // }

               simparams.simSpeedsFromTimeState( tmst_fpath );
               simparams.speedstepsFromSSprof();
            }
         }
         else
         {
            qDebug() << "SimMdl: timestate file does not exist";
            if ( !simparams.sim )
            { // Compute acceleration rate for non-astfem_sim data
               const double tfac  = ( 4.0 / 3.0 );
               double       t2    = simparams.speed_step[0].time_first;
               double       w2t   = simparams.speed_step[0].w2t_first;
               double       om1t  = simparams.speed_step[0].rotorspeed * M_PI / 30.0;
               double       w2    = sq( om1t );
               double       t1    = tfac * ( t2 - ( w2t / w2 ) );
               int          t_acc = (int) qRound( t1 );
               double       rate  = (double) ( simparams.speed_step[0].rotorspeed ) / (double) t_acc;
               qDebug() << "SimMdl:  accel-calc:  t1 t2 w2t t_acc speed rate" << t1 << t2 << w2t << t_acc
                        << simparams.speed_step[0].rotorspeed << rate;
               simparams.speed_step[0].acceleration = (int) qRound( rate );
            }
         }
         QStringList check_results = US_AstfemMath::check_acceleration( simparams.speed_step, edata->scanData );
         return check_results;
      }
};

// FEMatch's split of a model among threads and its sum of their results;
// the workers run here one after the other.
struct FrozenFeMatchThreads
{
      US_DataIO::RawData*     sdata = nullptr;
      US_SimulationParameters simparams;
      US_Model                model;
      struct
      {
            US_Buffer buffer;
      } solution_rec;
      double                    compress = 0.0;
      bool                      manual   = false;
      int                       nthread  = 0;
      int                       ncomp    = 0;
      int                       thrdone  = 0;
      QVector<int>              kcomps;
      QList<US_DataIO::RawData> tsimdats;
      QList<US_Model>           tmodels;
      void                      run()
      {
         solution_rec.buffer.compressibility = compress;
         solution_rec.buffer.manual          = manual;
         tsimdats.clear();
         tmodels.clear();
         kcomps.clear();
         QList<US_ThreadWorker*> tworkers;
         QList<US_ThreadEx*>     wthreads;

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

         thrdone                    = 0;
         solution_rec.buffer.manual = manual;
         for ( int ii = 0; ii < nthread; ii++ )
         {
            US_ThreadWorker tworker( tmodels[ii], simparams, tsimdats[ii], solution_rec.buffer, ii );
            tworker.calc_simulation();
         }
         for ( int ii = 0; ii < sdata->scanData.size(); ii++ )
         {
            for ( int jj = 0; jj < sdata->xvalues.size(); jj++ )
            {
               //double conc = 0.0;
               double conc = sdata->value( ii, jj );

               for ( int kk = 0; kk < nthread; kk++ )
               {
                  conc += tsimdats[kk].value( ii, jj );
               }

               sdata->setValue( ii, jj, conc );
            }
         }
      }
};
// An edited data set of one speed, its scans' times and omega-square-t consistent
US_DataIO::EditedData makeEdited( std::mt19937& rng, bool simulated )
{
   std::uniform_int_distribution<int>     nscan( 4, 16 ), npts( 20, 80 ), speedx( 0, 3 ), first( 300, 900 ), gap( 120, 600 );
   std::uniform_real_distribution<double> val( 0.0, 1.0 );
   const int                              speeds[] = { 20000, 40000, 50000, 60000 };
   const int                              rpm      = speeds[speedx( rng )];
   const double                           w2       = sq( rpm * M_PI / 30.0 );
   US_DataIO::EditedData                  e;
   e.runID       = "simrun";
   e.editID      = "1";
   e.dataType    = "RA";
   e.cell        = "1";
   e.channel     = simulated ? "S" : "A";
   e.wavelength  = "260";
   e.description = "test data";
   e.meniscus    = 5.9;
   e.bottom      = val( rng ) < 0.5 ? 7.2036 : 0.0;
   const int npt = npts( rng );
   for ( int jj = 0; jj < npt; jj++ )
   {
      e.xvalues << 6.0 + 1.1 * jj / ( npt - 1 );
   }
   int t = first( rng );
   for ( int ii = 0, n = nscan( rng ); ii < n; ii++ )
   {
      US_DataIO::Scan s;
      s.seconds     = t;
      s.rpm         = rpm;
      s.omega2t     = w2 * ( t - 150.0 );
      s.temperature = 20.0;
      s.wavelength  = 260.0;
      for ( int jj = 0; jj < npt; jj++ )
      {
         s.rvalues << val( rng );
      }
      e.scanData << s;
      t += gap( rng );
   }
   return e;
}

// A time state at 1-second intervals, as the programs write one
void writeOneSecond( const QString& path, US_DataIO::EditedData edata )
{
   US_SimulationParameters sp;
   sp.initFromData( nullptr, edata, true );
   sp.sim = ( edata.channel == "S" );
   US_DataIO::RawData sd;
   US_AstfemMath::initSimData( sd, edata, 0.0 );
   ASSERT_GT( US_AstfemMath::writetimestate( path, sp, sd ), 0 );
}

// A time state at 2-second intervals, which the programs replace
void writeTwoSecond( const QString& path, US_DataIO::EditedData& edata )
{
   const double rpm  = edata.scanData[0].rpm;
   const int    last = (int) edata.scanData.last().seconds + 10;
   US_TimeState ts;
   ASSERT_EQ( ts.open_write_data( path, 2.0, 0.0 ), 0 );
   for ( const char* key : { "Time:I4", "RawSpeed:F4", "SetSpeed:I4", "Omega2T:F4", "Temperature:F4", "Step:I2", "Scan:I2" } )
   {
      ts.set_key( QString( key ).section( ':', 0, 0 ), QString( key ).section( ':', 1, 1 ) );
   }
   double w2t = 0.0;
   for ( int t = 0; t <= last; t += 2 )
   {
      const double speed = qMin( rpm, 400.0 * t );
      w2t += 2.0 * sq( speed * M_PI / 30.0 );
      ts.set_value( "Time", t );
      ts.set_value( "RawSpeed", speed );
      ts.set_value( "SetSpeed", (int) rpm );
      ts.set_value( "Omega2T", w2t );
      ts.set_value( "Temperature", 20.0 );
      ts.set_value( "Step", 1 );
      ts.set_value( "Scan", 0 );
      ts.flush_record();
   }
   ts.close_write_data();
   ASSERT_EQ( ts.write_defs( 2.0 ), 0 );
}

QString dumpParms( US_SimulationParameters& sp, const QString& dir )
{
   const QString file = dir + "/parms.xml";
   sp.save_simparms( file );
   QFile f( file );
   f.open( QIODevice::ReadOnly );
   QString s = QString::fromUtf8( f.readAll() );
   f.remove();
   s += QString( "\nsim %1 band %2 bandvol %3 first %4 rcal %5 coeffs %6 %7 tsobj %8 res %9" )
           .arg( sp.sim )
           .arg( sp.band_forming )
           .arg( sp.band_volume, 0, 'g', 17 )
           .arg( sp.firstScanIsConcentration )
           .arg( sp.rotorCalID )
           .arg( sp.rotorcoeffs[0], 0, 'g', 17 )
           .arg( sp.rotorcoeffs[1], 0, 'g', 17 )
           .arg( sp.tsobj != nullptr )
           .arg( sp.radial_resolution, 0, 'g', 17 );
   for ( const auto& p : sp.sim_speed_prof )
   {
      // time_e_step is left out: SimSpeedProf's constructor does not set it
      s += QString( "\nssp %1 %2 %3 %4 %5 %6 %7 %8" )
              .arg( p.acceleration, 0, 'g', 17 )
              .arg( p.w2t_b_accel, 0, 'g', 17 )
              .arg( p.w2t_e_accel, 0, 'g', 17 )
              .arg( p.w2t_e_step, 0, 'g', 17 )
              .arg( p.rotorspeed )
              .arg( p.duration )
              .arg( p.time_b_accel )
              .arg( p.time_e_accel );
   }
   for ( const auto& p : sp.speed_step )
   {
      s += QString( "\nstep %1 %2 %3 %4 %5" )
              .arg( p.acceleration )
              .arg( p.time_first )
              .arg( p.time_last )
              .arg( p.w2t_first, 0, 'g', 17 )
              .arg( p.w2t_last, 0, 'g', 17 );
   }
   return s;
}

QString dumpData( const US_DataIO::RawData& d )
{
   QCryptographicHash h( QCryptographicHash::Sha256 );
   for ( double x : d.xvalues )
   {
      h.addData( QByteArray( (const char*) &x, sizeof x ) );
   }
   for ( const auto& s : d.scanData )
   {
      for ( double v : { s.seconds, s.omega2t, s.rpm, s.temperature } )
      {
         h.addData( QByteArray( (const char*) &v, sizeof v ) );
      }
      for ( double v : s.rvalues )
      {
         h.addData( QByteArray( (const char*) &v, sizeof v ) );
      }
   }
   return QString( "%1 %2 %3 %4 %5" )
      .arg( d.cell )
      .arg( d.channel )
      .arg( d.description )
      .arg( d.scanData.size() )
      .arg( QString( h.result().toHex() ) );
}

// Every file under the results folder, with its content hash
QString dumpFiles( const QString& dir )
{
   QStringList  out;
   QDirIterator it( dir, QDir::Files, QDirIterator::Subdirectories );
   while ( it.hasNext() )
   {
      const QString path = it.next();
      QFile         f( path );
      f.open( QIODevice::ReadOnly );
      out << QDir( dir ).relativeFilePath( path ) + " " +
                QCryptographicHash::hash( f.readAll(), QCryptographicHash::Sha256 ).toHex();
   }
   out.sort();
   return out.join( "\n" );
}

struct SetupCase
{
      US_DataIO::EditedData                          edata;
      US_Model                                       model;
      QMap<QString, QString>                         adv_vals;
      QVector<US_SimulationParameters::SpeedProfile> speed_steps;
      int                                            timestate         = 0; // 0 none, 1 at 1 second, 2 at 2 seconds
      bool                                           fematch_exp_steps = false;
      int                                            drow              = 0;
};

SetupCase randomSetup( std::mt19937& rng )
{
   std::uniform_int_distribution<int> coin( 0, 1 ), three( 0, 2 ), four( 0, 3 ), simpts( 50, 300 );
   SetupCase                          c;
   c.edata = makeEdited( rng, four( rng ) == 0 );
   US_Model::SimulationComponent sc;
   sc.s     = 4e-13;
   sc.D     = 6e-7;
   sc.sigma = four( rng ) == 0 ? 0.01 : 0.0;
   sc.delta = four( rng ) == 0 ? 0.02 : 0.0;
   c.model.components << sc << sc;
   c.adv_vals["simpoints"]  = QString::number( simpts( rng ) );
   c.adv_vals["gridtype"]   = coin( rng ) ? "Moving" : "Constant";
   c.adv_vals["bandvolume"] = "0.015";
   US_SimulationParameters::computeSpeedSteps( &c.edata.scanData, c.speed_steps );
   switch ( three( rng ) )
   {
      case 1:
         c.speed_steps[0].time_first += 100;
         break; // not covering the scans
      case 2:
         c.speed_steps[0].acceleration = 250;
         break; // differs from the data's
      default:
         break;
   }
   c.timestate         = three( rng );
   c.fematch_exp_steps = coin( rng );
   c.drow              = coin( rng );
   return c;
}

// A fresh results folder holding the case's time state, if any
QString freshResults( const QTemporaryDir& root, const QString& name, SetupCase& c )
{
   const QString base = root.path() + "/" + name;
   US_Settings::set_workBaseDir( base );
   const QString run = US_Settings::resultDir() + "/" + c.edata.runID;
   QDir().mkpath( run );
   const QString tmst = run + "/" + c.edata.runID + ".time_state.tmst";
   if ( c.timestate == 1 )
   {
      writeOneSecond( tmst, c.edata );
   }
   else if ( c.timestate == 2 )
   {
      writeTwoSecond( tmst, c.edata );
   }
   return tmst;
}

struct Outcome
{
      QString parms, data, results, files;
};

template <class Frozen> Outcome runFrozen( Frozen& f, SetupCase c, const QTemporaryDir& root, const QString& name )
{
   freshResults( root, name, c );
   US_DataIO::RawData sdata;
   f.edata                   = &c.edata;
   f.sdata                   = &sdata;
   f.adv_vals                = c.adv_vals;
   f.speed_steps             = c.speed_steps;
   f.model                   = c.model;
   const QStringList results = f.run();
   return { dumpParms( f.simparams, root.path() ), dumpData( sdata ), results.join( "|" ), dumpFiles( US_Settings::resultDir() ) };
}

Outcome runShared( SetupCase c, bool exp_steps, bool rewrite, const QTemporaryDir& root, const QString& name )
{
   const QString           tmst = freshResults( root, name, c );
   US_DataIO::RawData      sdata;
   US_SimulationParameters simparams;
   const QStringList       results = US_ModelSimulation::prepare(
      nullptr, -1, c.edata, c.model, c.adv_vals, exp_steps, c.speed_steps, tmst, rewrite, simparams, sdata );
   return { dumpParms( simparams, root.path() ), dumpData( sdata ), results.join( "|" ), dumpFiles( US_Settings::resultDir() ) };
}

void expectSame( const Outcome& a, const Outcome& b )
{
   EXPECT_EQ( a.parms, b.parms );
   EXPECT_EQ( a.data, b.data );
   EXPECT_EQ( a.results, b.results );
   EXPECT_EQ( a.files, b.files );
}

// The threads the process has now; -1 where the test cannot count them
int processThreads()
{
#if defined( Q_OS_MACOS )
   thread_act_array_t     list;
   mach_msg_type_number_t count = 0;
   if ( task_threads( mach_task_self(), &list, &count ) != KERN_SUCCESS )
   {
      return -1;
   }
   for ( mach_msg_type_number_t i = 0; i < count; i++ )
   {
      mach_port_deallocate( mach_task_self(), list[i] );
   }
   vm_deallocate( mach_task_self(), (vm_address_t) list, count * sizeof( thread_act_t ) );
   return (int) count;
#elif defined( Q_OS_LINUX )
   QFile status( "/proc/self/status" );
   if ( !status.open( QIODevice::ReadOnly ) )
   {
      return -1;
   }
   for ( const QByteArray& line : status.readAll().split( '\n' ) )
   {
      if ( line.startsWith( "Threads:" ) )
      {
         return line.mid( 8 ).trimmed().toInt();
      }
   }
   return -1;
#else
   return -1;
#endif
}

// A model of n ideal components with spread s values
US_Model manyComponents( int n )
{
   US_Model m;
   for ( int ii = 0; ii < n; ii++ )
   {
      US_Model::SimulationComponent sc;
      sc.s                    = ( 1.0 + 0.2 * ii ) * 1e-13;
      sc.D                    = ( 8.0 - 0.05 * ii ) * 1e-7;
      sc.signal_concentration = 0.01 + 0.001 * ii;
      m.components << sc;
   }
   return m;
}
} // namespace

class TestUSModelSimulation : public QtTestBase
{
};

// The parameters, the simulated data's time grid, the acceleration check and
// the time state files are as each program's old code left them.
TEST_F( TestUSModelSimulation, PrepareMatchesTheThreeOldCopies )
{
   const QString saved = US_Settings::workBaseDir();
   QTemporaryDir root;
   std::mt19937  rng( 1243 );
   int           cases = 0, replaced = 0, rewriteSkipped = 0, finiteVolume = 0, checkMessages = 0;
   for ( int n = 0; n < 120; n++ )
   {
      SetupCase c = randomSetup( rng );
      SCOPED_TRACE( n );
      const bool         cover = US_SimulationParameters::speedStepsCoverData( c.speed_steps, c.edata );

      FrozenFeMatchSetup fe;
      fe.runID            = c.edata.runID;
      fe.drow             = c.drow;
      fe.exp_steps        = c.fematch_exp_steps;
      fe.dat_steps        = !c.fematch_exp_steps;
      const Outcome feOld = runFrozen( fe, c, root, QString( "fe-old-%1" ).arg( n ) );
      const Outcome feNew = runShared( c, c.fematch_exp_steps, c.drow == 0, root, QString( "fe-new-%1" ).arg( n ) );
      expectSame( feOld, feNew );

      FrozenAutoflowSetup au;
      au.FileName_parsed  = c.edata.runID;
      const Outcome auOld = runFrozen( au, c, root, QString( "au-old-%1" ).arg( n ) );
      const Outcome auNew = runShared( c, cover, true, root, QString( "au-new-%1" ).arg( n ) );
      expectSame( auOld, auNew );

      FrozenReporterSetup re;
      re.FileName_parsed  = c.edata.runID;
      const Outcome reOld = runFrozen( re, c, root, QString( "re-old-%1" ).arg( n ) );
      const Outcome reNew = runShared( c, cover, true, root, QString( "re-new-%1" ).arg( n ) );
      expectSame( reOld, reNew );

      cases++;
      replaced += c.timestate == 2;
      rewriteSkipped += c.timestate == 2 && c.drow != 0;
      finiteVolume += c.model.components[0].sigma != 0.0 || c.model.components[0].delta != 0.0;
      checkMessages += !auNew.results.isEmpty();
   }
   US_Settings::set_workBaseDir( saved );
   EXPECT_EQ( cases, 120 );
   EXPECT_GT( replaced, 20 );
   EXPECT_GT( rewriteSkipped, 5 );
   EXPECT_GT( finiteVolume, 10 );
   RecordProperty( "acceleration_messages", checkMessages );
}

// Split among threads and added up, the simulation is FEMatch's to the bit.
TEST_F( TestUSModelSimulation, ThreadsMatchFeMatchsSplitAndSum )
{
   const QString saved = US_Settings::workBaseDir();
   QTemporaryDir root;
   US_Settings::set_workBaseDir( root.path() );
   std::mt19937           rng( 7 );
   US_DataIO::EditedData  edata = makeEdited( rng, false );
   QMap<QString, QString> adv{ { "simpoints", "100" }, { "gridtype", "Moving" }, { "bandvolume", "0.015" } };

   for ( int nthread : { 2, 3 } )
   {
      SCOPED_TRACE( nthread );
      const US_Model          model = manyComponents( 26 * nthread + 1 );
      US_SimulationParameters simparams;
      US_DataIO::RawData      grid;
      US_ModelSimulation::prepare( nullptr, -1, edata, model, adv, false, {}, root.path() + "/none.tmst", true, simparams, grid );

      FrozenFeMatchThreads old;
      US_DataIO::RawData   oldData = grid;
      old.sdata                    = &oldData;
      old.simparams                = simparams;
      old.model                    = model;
      old.ncomp                    = model.components.size();
      old.nthread                  = nthread;
      old.run();

      US_DataIO::RawData newData = grid;
      US_Model           m       = model;
      US_Buffer          buffer;
      US_ModelSimulation sim;
      QEventLoop         loop;
      int                lastProgress = 0;
      QObject::connect( &sim, &US_ModelSimulation::progress, [&]( int k ) { lastProgress = k; } );
      QObject::connect( &sim, &US_ModelSimulation::finished, &loop, &QEventLoop::quit );
      QTimer::singleShot( 300000, &loop, &QEventLoop::quit );
      sim.start( m, simparams, newData, buffer, 0.0, false, nthread );
      loop.exec();

      EXPECT_EQ( dumpData( newData ), dumpData( oldData ) );
      EXPECT_EQ( lastProgress, -nthread ); // each worker ends by reporting -1, as FEMatch summed them
      double total = 0.0;
      for ( const auto& s : newData.scanData )
      {
         for ( double v : s.rvalues )
         {
            total += v;
         }
      }
      EXPECT_GT( total, 0.0 );
   }
   US_Settings::set_workBaseDir( saved );
}

// In one thread: the finite element solver for an ideal model, otherwise
// finite volume, as the programs ran them
TEST_F( TestUSModelSimulation, SimulatesInThisThreadWithTheOldSolvers )
{
   const QString saved = US_Settings::workBaseDir();
   QTemporaryDir root;
   US_Settings::set_workBaseDir( root.path() );
   std::mt19937           rng( 11 );
   US_DataIO::EditedData  edata = makeEdited( rng, false );
   QMap<QString, QString> adv{ { "simpoints", "100" }, { "gridtype", "Moving" }, { "bandvolume", "0.015" } };

   for ( double sigma : { 0.0, 0.01 } )
   {
      SCOPED_TRACE( sigma );
      US_Model model            = manyComponents( 3 );
      model.components[0].sigma = sigma;
      US_SimulationParameters simparams;
      US_DataIO::RawData      grid;
      US_ModelSimulation::prepare( nullptr, -1, edata, model, adv, false, {}, root.path() + "/none.tmst", true, simparams, grid );

      // The old code: a new solver of the same kind, given no buffer
      US_DataIO::RawData oldData = grid;
      if ( sigma == 0.0 )
      {
         US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
         astfem_rsa->set_debug_flag( 0 );
         astfem_rsa->calculate( oldData );
      }
      else
      {
         US_LammAstfvm* astfvm = new US_LammAstfvm( model, simparams );
         astfvm->calculate( oldData );
      }

      US_DataIO::RawData newData = grid;
      US_Buffer          buffer;
      buffer.compressibility = 9.0;
      buffer.manual          = true;
      US_ModelSimulation sim;
      int                components = 0;
      QObject::connect( &sim, &US_ModelSimulation::component, [&]( int ) { components++; } );
      sim.simulate( model, simparams, newData, buffer, 0.0, false, 0 );

      EXPECT_EQ( dumpData( newData ), dumpData( oldData ) );
      // The finite element path sets the buffer and reports each component
      EXPECT_EQ( buffer.compressibility, sigma == 0.0 ? 0.0 : 9.0 );
      EXPECT_EQ( buffer.manual, sigma != 0.0 );
      EXPECT_EQ( components, sigma == 0.0 ? 4 : 0 ); // once per component and once at the end
   }
   US_Settings::set_workBaseDir( saved );
}

// The worker threads stop and are deleted once their results are added.
TEST_F( TestUSModelSimulation, WorkerThreadsEndWithTheSimulation )
{
   if ( processThreads() < 0 )
   {
      GTEST_SKIP() << "no thread count on this platform";
   }
   const QString saved = US_Settings::workBaseDir();
   QTemporaryDir root;
   US_Settings::set_workBaseDir( root.path() );
   std::mt19937            rng( 13 );
   US_DataIO::EditedData   edata = makeEdited( rng, false );
   QMap<QString, QString>  adv{ { "simpoints", "50" }, { "gridtype", "Moving" }, { "bandvolume", "0.015" } };
   const US_Model          model = manyComponents( 2 * 26 + 1 );
   US_SimulationParameters simparams;
   US_DataIO::RawData      grid;
   US_ModelSimulation::prepare( nullptr, -1, edata, model, adv, false, {}, root.path() + "/none.tmst", true, simparams, grid );

   US_ModelSimulation sim;
   QEventLoop         loop;
   QObject::connect( &sim, &US_ModelSimulation::finished, &loop, &QEventLoop::quit );
   const int before = processThreads();
   for ( int run = 0; run < 3; run++ )
   {
      US_DataIO::RawData data = grid;
      US_Model           m    = model;
      US_Buffer          buffer;
      QTimer::singleShot( 300000, &loop, &QEventLoop::quit );
      sim.start( m, simparams, data, buffer, 0.0, false, 2 );
      loop.exec();
   }
   // Let finished threads end and deferred deletes run
   QElapsedTimer wait;
   wait.start();
   while ( processThreads() > before && wait.elapsed() < 10000 )
   {
      QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
      QCoreApplication::processEvents( QEventLoop::AllEvents, 50 );
      QThread::msleep( 20 );
   }
   EXPECT_LE( processThreads(), before ) << "six worker threads were started";
   US_Settings::set_workBaseDir( saved );
}

// A co-sedimenting model whose first component also has sigma is refused
// before the salt data is made; deleting the solver must not free the salt
// data pointer it never set.
TEST_F( TestUSModelSimulation, RefusedCoSedimentingSolverIsDeletedSafely )
{
   const QString saved = US_Settings::workBaseDir();
   QTemporaryDir root;
   US_Settings::set_workBaseDir( root.path() );
   std::mt19937           rng( 17 );
   US_DataIO::EditedData  edata = makeEdited( rng, false );
   QMap<QString, QString> adv{ { "simpoints", "50" }, { "gridtype", "Moving" }, { "bandvolume", "0.015" } };
   US_Model               model = manyComponents( 2 );
   model.coSedSolute            = 0;
   model.components[0].sigma    = 0.01;
   US_SimulationParameters simparams;
   US_DataIO::RawData      grid;
   US_ModelSimulation::prepare( nullptr, -1, edata, model, adv, false, {}, root.path() + "/none.tmst", true, simparams, grid );

   // Build the solver over memory that is not zero, as the stack can be
   alignas( US_LammAstfvm ) unsigned char storage[sizeof( US_LammAstfvm )];
   std::memset( storage, 0x5a, sizeof storage );
   US_LammAstfvm*     astfvm = new ( storage ) US_LammAstfvm( model, simparams );
   US_DataIO::RawData data   = grid;
   EXPECT_NE( astfvm->calculate( data ), 0 );
   astfvm->~US_LammAstfvm();

   // Through the shared code, in this thread
   US_DataIO::RawData data2 = grid;
   US_Buffer          buffer;
   US_ModelSimulation sim;
   sim.simulate( model, simparams, data2, buffer, 0.0, false, 0 );
   US_Settings::set_workBaseDir( saved );
}
