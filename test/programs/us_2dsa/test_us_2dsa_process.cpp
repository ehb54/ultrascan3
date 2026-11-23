//! \file test_us_2dsa_process.cpp
//! \brief Meniscus-scan models of US_2dsaProcess against separate fits.
//!
//! A meniscus scan fits one model per meniscus value, and requeue_tasks()
//! sets up every model after the first.  Each model must equal a separate fit
//! at its meniscus value, bit for bit, since the fits run in one thread.  The
//! data are simulated as test/mpi_analysis/us_mpi_testdata.cpp simulates them,
//! with time- and radially-invariant noise added.  The fits refine iteratively
//! and fit both noises, so a model that inherits the previous model's
//! refinement records, noise setting or task size limit differs.  Every model
//! must also start from the progress estimate that the first started from.

#include <QEventLoop>
#include <QTemporaryDir>
#include <QTimer>
#include <cmath>
#include <cstring>

#include "us3_gui_test_main.h"
#include "us_2dsa_process.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_dataIO.h"
#include "us_math2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_simparms.h"
#include "us_solve_sim.h"
#include "us_util.h"

namespace
{
// Experiment and sample as in test/mpi_analysis/us_mpi_testdata.cpp, with
//  fewer scans and radial points
const double kRpm        = 60000.0;
const double kAccelRate  = 400.0; // rpm per second
const double kMeniscus   = 5.90;  // meniscus of the simulation
const double kEditMenis  = 5.905; // meniscus of the edit, the scan's center
const double kCpBottom   = 7.20;
const double kStretch0   = 0.0;
const double kStretch1   = 1.0e-12;
const double kDensity    = 0.998234;
const double kViscosity  = 1.001940;
const double kVbar20     = 0.72;
const double kRadiusLo   = 5.95;
const double kRadiusHi   = 7.15;
const double kRadiusStep = 0.012;
const int    kScans      = 12;
const double kSigma      = 0.004; // random noise
const double kTiAmp      = 0.02;  // time-invariant noise amplitude
const double kRiAmp      = 0.01;  // radially-invariant noise amplitude
const uint   kSeed       = 4242;

// Fit:  a 24 x 24 grid, up to three refinement iterations, both noises
const double kSLow        = 1.0;
const double kSHigh       = 10.0;
const double kKLow        = 1.0;
const double kKHigh       = 4.0;
const int    kGridS       = 24;
const int    kGridK       = 24;
const int    kMaxIters    = 3;
const int    kNoise       = 3; // time- and radially-invariant
const int    kMeniscusPts = 3;
const double kMeniscusRng = 0.03;
const int    kFitTimeout  = 600000; // ms

// What a fit returned for one of its models
struct FitModel
{
      QString            rounds; // refinement iterations
      US_Model           model;
      US_Noise           ti_noise;
      US_Noise           ri_noise;
      US_DataIO::RawData residuals;
};

// A fit's models, and the progress estimate each model started from
struct FitRun
{
      QList<FitModel> models;
      QList<int>      estimates;
};

double stretched_bottom()
{
   double stretch[2] = { kStretch0, kStretch1 };
   return US_AstfemMath::calc_bottom( kRpm, kCpBottom, stretch );
}

US_DataIO::RawData skeleton()
{
   US_DataIO::RawData raw;
   memcpy( raw.type, "RI", 2 );
   raw.cell        = 1;
   raw.channel     = 'A';
   raw.description = "2DSA process test data";
   US_Util::uuid_parse( "00000000-0000-4000-8000-000000002d5a", (uchar*) raw.rawGUID );

   const int npoints = qRound( ( kRadiusHi - kRadiusLo ) / kRadiusStep ) + 1;
   for ( int rr = 0; rr < npoints; rr++ )
   {
      raw.xvalues << kRadiusLo + rr * kRadiusStep;
   }

   const double omega = kRpm * M_PI / 30.0;
   const double t_acc = kRpm / kAccelRate;
   for ( int ss = 0; ss < kScans; ss++ )
   {
      US_DataIO::Scan scan;
      scan.temperature  = 20.0;
      scan.rpm          = kRpm;
      scan.seconds      = 600.0 + 480.0 * ss;
      scan.omega2t      = omega * omega * ( scan.seconds - 2.0 * t_acc / 3.0 );
      scan.wavelength   = 260.0;
      scan.plateau      = 0.0;
      scan.delta_r      = kRadiusStep;
      scan.nz_stddev    = false;
      scan.interpolated = QByteArray( ( npoints + 7 ) / 8, '\0' );
      scan.rvalues.fill( 0.0, npoints );
      raw.scanData << scan;
   }

   return raw;
}

// Builds the dataset as US_2dsa::prep_fit_dataset() does, from data files
//  written and loaded as us_mpi_testdata writes them and us_2dsa loads them
QString make_dataset( const QString& work, US_SolveSim::DataSet& dset )
{
   const QString         runID    = "test2dsa";
   const QString         aucname  = runID + ".RI.1.A.260.auc";
   const QString         editname = runID + ".e1.RI.1.A.260.xml";
   US_DataIO::RawData    raw      = skeleton();

   US_DataIO::EditValues ev;
   ev.expType    = "Velocity";
   ev.runID      = runID;
   ev.cell       = "1";
   ev.channel    = "A";
   ev.wavelength = "260";
   ev.editGUID   = "00000000-0000-4000-8000-000000002d5b";
   ev.dataGUID   = US_Util::uuid_unparse( (uchar*) raw.rawGUID );
   ev.meniscus   = kEditMenis;
   ev.bottom     = stretched_bottom();
   ev.rangeLeft  = 6.0;
   ev.rangeRight = 7.1;
   ev.plateau    = 7.0;
   ev.baseline   = 7.12;
   ev.ODlimit    = 1.5;

   US_DataIO::EditedData edata;
   if ( US_DataIO::writeRawData( work + "/" + aucname, raw ) != US_DataIO::OK ||
        US_DataIO::writeEdits( work + "/" + editname, ev ) != US_DataIO::OK ||
        US_DataIO::loadData( work, editname, edata ) != US_DataIO::OK )
   {
      return "cannot write or load the data files";
   }

   US_SimulationParameters sp;
   sp.initFromData( nullptr, edata, true );
   sp.rotorcoeffs[0]  = kStretch0;
   sp.rotorcoeffs[1]  = kStretch1;
   sp.bottom_position = kCpBottom;
   sp.bottom          = ( edata.bottom > 0.0 ) ? edata.bottom : stretched_bottom();
   sp.band_forming    = false;
   US_DataIO::RawData sdat;
   US_AstfemMath::initSimData( sdat, edata, 0.0 );
   if ( US_AstfemMath::timestate_from_speedsteps( sp, sdat ) < 0 )
   {
      return "cannot make the timestate";
   }
   sp.speedstepsFromSSprof();

   const double           avtemp = edata.average_temperature();
   US_Math2::SolutionData sd;
   sd.density   = kDensity;
   sd.viscosity = kViscosity;
   sd.manual    = false;
   sd.vbar20    = kVbar20;
   sd.vbar      = kVbar20;
   US_Math2::data_correction( avtemp, sd );

   // Three species (s 20,W, f/f0, signal), simulated in experiment space
   const double species[3][3] = { { 2.5, 1.25, 0.25 }, { 4.5, 1.6, 0.40 }, { 7.0, 2.1, 0.30 } };
   US_Model     model;
   for ( int ii = 0; ii < 3; ii++ )
   {
      US_Model::SimulationComponent comp;
      comp.s                    = species[ii][0] * 1.0e-13;
      comp.f_f0                 = species[ii][1];
      comp.vbar20               = kVbar20;
      comp.signal_concentration = species[ii][2];
      model.components << comp;
   }
   model.update_coefficients();
   for ( int ii = 0; ii < model.components.size(); ii++ )
   {
      model.components[ii].s /= sd.s20w_correction;
      model.components[ii].D /= sd.D20w_correction;
   }

   US_SimulationParameters sp_true = sp;
   sp_true.meniscus                = kMeniscus;
   US_DataIO::RawData simdat;
   US_AstfemMath::initSimData( simdat, edata, 0.0 );
   US_Astfem_RSA astfem( model, sp_true );
   astfem.calculate( simdat );

   // Data = simulation + random noise + time- and radially-invariant noise
   US_Math2::randomize( kSeed );
   for ( int ss = 0; ss < edata.scanCount(); ss++ )
   {
      const double rinoi = kRiAmp * ( (double) ( ( ss * 7 ) % 11 ) - 5.0 ) / 5.0;
      for ( int rr = 0; rr < edata.pointCount(); rr++ )
      {
         const double rad   = edata.radius( rr );
         const double tinoi = kTiAmp * ( 0.5 + 0.5 * sin( 2.0 * M_PI * ( rad - 6.0 ) / 0.37 ) );
         edata.setValue( ss, rr, simdat.value( ss, rr ) + tinoi + rinoi + US_Math2::box_muller( 0.0, kSigma ) );
      }
   }

   dset.requestID          = "test";
   dset.run_data           = edata;
   dset.simparams          = sp;
   dset.viscosity          = kViscosity;
   dset.density            = kDensity;
   dset.compress           = 0.0;
   dset.temperature        = avtemp;
   dset.vbar20             = kVbar20;
   dset.vbartb             = kVbar20;
   dset.s20w_correction    = sd.s20w_correction;
   dset.D20w_correction    = sd.D20w_correction;
   dset.rotor_stretch[0]   = kStretch0;
   dset.rotor_stretch[1]   = kStretch1;
   dset.centerpiece_bottom = kCpBottom;
   dset.zcoeffs[0]         = 0.0;
   dset.zcoeffs[1]         = 0.0;
   dset.zcoeffs[2]         = 0.0;
   dset.zcoeffs[3]         = 0.0;
   dset.solute_type        = 0;
   dset.manual             = false;
   return QString();
}

// Runs a fit in one thread, a meniscus scan or a separate fit at a given
//  meniscus value, and returns its models in order (none if it failed)
FitRun run_fit( const US_SolveSim::DataSet& base, bool scan, double meniscus = 0.0 )
{
   US_SolveSim::DataSet dset = base;
   if ( !scan )
   {
      dset.run_data.meniscus  = meniscus;
      dset.simparams.meniscus = meniscus;
   }
   QList<US_SolveSim::DataSet*> dsets;
   dsets << &dset;

   FitRun         run;
   US_2dsaProcess proc( dsets );
   QEventLoop     loop;
   bool           done        = false;
   bool           model_start = true;

   // Stage 0 ends a refinement iteration, 1 a scan model before the last,
   //  9 the fit;  anything else stops it
   QObject::connect( &proc,
                     &US_2dsaProcess::process_complete,
                     &loop,
                     [&]( int stage )
                     {
                        if ( stage == 1 || stage == 9 )
                        {
                           FitModel               fm;
                           US_DataIO::RawData     sim;
                           QMap<QString, QString> values;
                           proc.get_results( &sim, &fm.residuals, &fm.model, &fm.ti_noise, &fm.ri_noise );
                           proc.get_values( values );
                           fm.rounds = values["rf_iteration"];
                           run.models << fm;
                           model_start = true;
                        }
                        if ( stage != 0 && stage != 1 )
                        {
                           done = ( stage == 9 );
                           loop.quit();
                        }
                     } );

   // A model's first progress report gives the estimate it starts from
   QObject::connect( &proc,
                     &US_2dsaProcess::stage_complete,
                     &loop,
                     [&]( int, int total )
                     {
                        if ( model_start )
                        {
                           run.estimates << total;
                           model_start = false;
                        }
                     } );

   int nss = kGridS;
   int nks = kGridK;
   int ngr = US_Math2::best_grid_reps( nss, nks );
   proc.set_iters( kMaxIters, 0, scan ? kMeniscusPts : 0, 1.0e-12, kMeniscusRng, 0.0, ngr, scan ? 1 : 0 );
   proc.start_fit( kSLow, kSHigh, nss, kKLow, kKHigh, nks, ngr, 1, kNoise );
   QTimer::singleShot( kFitTimeout, &loop, &QEventLoop::quit );
   loop.exec();

   if ( !done )
   { // Timed out or stopped:  end any work before the processor goes
      proc.stop_fit();
      run.models.clear();
   }
   return run;
}

// The first difference between two models, or an empty string
QString difference( const FitModel& aa, const FitModel& bb )
{
   if ( aa.rounds != bb.rounds )
   {
      return QString( "refinement iterations %1 vs %2" ).arg( aa.rounds, bb.rounds );
   }

   const QVector<US_Model::SimulationComponent>& ca = aa.model.components;
   const QVector<US_Model::SimulationComponent>& cb = bb.model.components;
   if ( ca.size() != cb.size() )
   {
      return QString( "%1 vs %2 solutes" ).arg( ca.size() ).arg( cb.size() );
   }
   for ( int cc = 0; cc < ca.size(); cc++ )
   {
      if ( ca[cc].s != cb[cc].s || ca[cc].D != cb[cc].D || ca[cc].f_f0 != cb[cc].f_f0 ||
           ca[cc].signal_concentration != cb[cc].signal_concentration )
      {
         return QString( "solute %1: s %2 vs %3, f/f0 %4 vs %5, signal %6 vs %7" )
            .arg( cc + 1 )
            .arg( ca[cc].s, 0, 'g', 17 )
            .arg( cb[cc].s, 0, 'g', 17 )
            .arg( ca[cc].f_f0, 0, 'g', 17 )
            .arg( cb[cc].f_f0, 0, 'g', 17 )
            .arg( ca[cc].signal_concentration, 0, 'g', 17 )
            .arg( cb[cc].signal_concentration, 0, 'g', 17 );
      }
   }

   if ( aa.ti_noise.values != bb.ti_noise.values )
   {
      return "time-invariant noise";
   }
   if ( aa.ri_noise.values != bb.ri_noise.values )
   {
      return "radially-invariant noise";
   }

   if ( aa.residuals.scanData.size() != bb.residuals.scanData.size() )
   {
      return "residual scan count";
   }
   for ( int ss = 0; ss < aa.residuals.scanData.size(); ss++ )
   {
      if ( aa.residuals.scanData[ss].rvalues != bb.residuals.scanData[ss].rvalues )
      {
         return QString( "residuals of scan %1" ).arg( ss + 1 );
      }
   }

   return QString();
}
} // namespace

class US_2dsaProcessTest : public QObject
{
      Q_OBJECT

   private slots:
      // The data, and the meniscus scan that both tests check
      void initTestCase()
      {
         QVERIFY( work.isValid() );
         const QString error = make_dataset( work.path(), base );
         QVERIFY2( error.isEmpty(), qPrintable( error ) );

         scan = run_fit( base, true );
         QCOMPARE( (int) scan.models.size(), kMeniscusPts );
      }

      // Each model's first iteration runs the same subgrids, so every model
      //  starts from the progress estimate of the first
      void scanModelsStartFromTheSameEstimate()
      {
         QCOMPARE( (int) scan.estimates.size(), kMeniscusPts );
         for ( int mm = 1; mm < kMeniscusPts; mm++ )
         {
            QCOMPARE( scan.estimates[mm], scan.estimates[0] );
         }
      }

      // Every model of the scan is the separate fit at its meniscus value
      void scanModelsEqualSeparateFits()
      {
         const double bmenis   = base.run_data.meniscus;
         const double mendelta = kMeniscusRng / (double) ( kMeniscusPts - 1 );
         for ( int mm = 0; mm < kMeniscusPts; mm++ )
         {
            // The meniscus value as US_2dsaProcess computes it for the scan
            const double menis  = ( bmenis - kMeniscusRng * 0.5 ) + ( (double) mm * mendelta );
            const FitRun single = run_fit( base, false, menis );
            QCOMPARE( (int) single.models.size(), 1 );

            // Both noises were fitted, and refinement iterated
            const FitModel& fit = single.models[0];
            QVERIFY( !fit.ti_noise.values.isEmpty() );
            QVERIFY( !fit.ri_noise.values.isEmpty() );
            QVERIFY( fit.rounds.toInt() >= 2 );

            const QString diff = difference( scan.models[mm], fit );
            QVERIFY2( diff.isEmpty(),
                      qPrintable( QString( "scan model %1 (meniscus %2) differs from the separate fit: %3" )
                                     .arg( mm + 1 )
                                     .arg( menis, 0, 'f', 5 )
                                     .arg( diff ) ) );
         }
      }

   private:
      QTemporaryDir        work;
      US_SolveSim::DataSet base;
      FitRun               scan;
};

US3_GUI_TEST_MAIN( US_2dsaProcessTest )

#include "test_us_2dsa_process.moc"
