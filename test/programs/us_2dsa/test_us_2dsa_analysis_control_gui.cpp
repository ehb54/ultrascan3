//! \file test_us_2dsa_analysis_control_gui.cpp
//! \brief The 2DSA analysis dialog keeps noise fitting off with Monte Carlo.
//!
//! Monte Carlo iterations fit no noise:  optimize_options() unchecks and
//! disables the noise boxes while Monte Carlo is checked.  The grid boxes
//! (Uniform Grid, Custom Grid) must not enable them again, or start() could
//! read Monte Carlo together with a noise flag.

#include <QCheckBox>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <cstring>

#include "us3_gui_test_main.h"
#include "us_analysis_control_2d.h"
#include "us_dataIO.h"
#include "us_simparms.h"
#include "us_solve_sim.h"
#include "us_util.h"

namespace
{
// A flat velocity dataset:  the dialog reads its dimensions and settings only
QString make_dataset( const QString& work, US_SolveSim::DataSet& dset )
{
   const QString      runID    = "test2dsadlg";
   const QString      aucname  = runID + ".RI.1.A.260.auc";
   const QString      editname = runID + ".e1.RI.1.A.260.xml";

   US_DataIO::RawData raw;
   memcpy( raw.type, "RI", 2 );
   raw.cell        = 1;
   raw.channel     = 'A';
   raw.description = "2DSA dialog test data";
   US_Util::uuid_parse( "00000000-0000-4000-8000-000000002d5c", (uchar*) raw.rawGUID );
   for ( int rr = 0; rr <= 200; rr++ )
   {
      raw.xvalues << 5.95 + rr * 0.006;
   }
   for ( int ss = 0; ss < 30; ss++ )
   {
      US_DataIO::Scan scan;
      scan.temperature  = 20.0;
      scan.rpm          = 60000.0;
      scan.seconds      = 600.0 + 240.0 * ss;
      scan.omega2t      = 6.2832e3 * 6.2832e3 * ( scan.seconds - 100.0 );
      scan.wavelength   = 260.0;
      scan.plateau      = 0.0;
      scan.delta_r      = 0.006;
      scan.nz_stddev    = false;
      scan.interpolated = QByteArray( ( raw.xvalues.size() + 7 ) / 8, '\0' );
      scan.rvalues.fill( 0.1, raw.xvalues.size() );
      raw.scanData << scan;
   }

   US_DataIO::EditValues ev;
   ev.expType    = "Velocity";
   ev.runID      = runID;
   ev.cell       = "1";
   ev.channel    = "A";
   ev.wavelength = "260";
   ev.editGUID   = "00000000-0000-4000-8000-000000002d5d";
   ev.dataGUID   = US_Util::uuid_unparse( (uchar*) raw.rawGUID );
   ev.meniscus   = 5.905;
   ev.bottom     = 7.2;
   ev.rangeLeft  = 6.0;
   ev.rangeRight = 7.1;
   ev.plateau    = 7.0;
   ev.baseline   = 7.12;
   ev.ODlimit    = 1.5;

   if ( US_DataIO::writeRawData( work + "/" + aucname, raw ) != US_DataIO::OK ||
        US_DataIO::writeEdits( work + "/" + editname, ev ) != US_DataIO::OK ||
        US_DataIO::loadData( work, editname, dset.run_data ) != US_DataIO::OK )
   {
      return "cannot write or load the data files";
   }

   dset.simparams.initFromData( nullptr, dset.run_data, true );
   dset.viscosity       = 1.001940;
   dset.density         = 0.998234;
   dset.compress        = 0.0;
   dset.temperature     = 20.0;
   dset.vbar20          = 0.72;
   dset.vbartb          = 0.72;
   dset.s20w_correction = 1.0;
   dset.D20w_correction = 1.0;
   dset.solute_type     = 0;
   dset.manual          = false;
   return QString();
}

QCheckBox* checkbox( QWidget* dialog, const QString& text )
{
   for ( QCheckBox* box : dialog->findChildren<QCheckBox*>() )
   {
      if ( box->text() == text )
      {
         return box;
      }
   }

   return nullptr;
}
} // namespace

class US_2dsaAnalysisControlGuiTest : public QObject
{
      Q_OBJECT

   private slots:
      void initTestCase()
      {
         QVERIFY( work.isValid() );
         const QString error = make_dataset( work.path(), dset );
         QVERIFY2( error.isEmpty(), qPrintable( error ) );
      }

      void monteCarloKeepsNoiseFittingOff()
      {
         QList<US_SolveSim::DataSet*>         dsets  = { &dset };
         bool                                 loadDB = false;
         QScopedPointer<US_AnalysisControl2D> dialog( new US_AnalysisControl2D( dsets, loadDB, nullptr ) );
         dialog->show();

         QCheckBox* ti = checkbox( dialog.data(), "Fit Time-Invariant Noise" );
         QCheckBox* ri = checkbox( dialog.data(), "Fit Radially-Invariant Noise" );
         QCheckBox* mc = checkbox( dialog.data(), "Monte Carlo Iterations" );
         QCheckBox* ug = checkbox( dialog.data(), "Uniform Grid" );
         QCheckBox* cg = checkbox( dialog.data(), "Custom Grid" );
         if ( ti == nullptr || ri == nullptr || mc == nullptr || ug == nullptr || cg == nullptr )
         {
            QFAIL( "The dialog has no checkbox with one of the expected labels" );
         }

         // What start() would read:  Monte Carlo together with a noise flag
         auto mc_with_noise = [&]() { return mc->isChecked() && ( ti->isChecked() || ri->isChecked() ); };

         QVERIFY( ug->isChecked() && !mc->isChecked() );
         QVERIFY( ti->isEnabled() && ri->isEnabled() );
         ti->click();
         QVERIFY( ti->isChecked() );

         // Monte Carlo unchecks and disables the noise boxes
         mc->click();
         QVERIFY( mc->isChecked() );
         QVERIFY( !ti->isChecked() && !ri->isChecked() );
         QVERIFY( !ti->isEnabled() && !ri->isEnabled() );

         // Neither grid box enables them again
         cg->click();
         QVERIFY( cg->isChecked() && !ug->isChecked() );
         QVERIFY( !ti->isEnabled() && !ri->isEnabled() );
         ti->click();
         QVERIFY( !mc_with_noise() );

         ug->click();
         QVERIFY( ug->isChecked() && !cg->isChecked() );
         QVERIFY( !ti->isEnabled() && !ri->isEnabled() );
         ri->click();
         QVERIFY( !mc_with_noise() );

         // Without Monte Carlo, both grids allow noise fitting again
         mc->click();
         QVERIFY( !mc->isChecked() );
         QVERIFY( ti->isEnabled() && ri->isEnabled() );
         ti->click();
         QVERIFY( ti->isChecked() );

         cg->click();
         QVERIFY( cg->isChecked() && !ug->isChecked() );
         QVERIFY( ti->isEnabled() && ri->isEnabled() );
         QVERIFY( ti->isChecked() );
      }

   private:
      QTemporaryDir        work;
      US_SolveSim::DataSet dset;
};

US3_GUI_TEST_MAIN( US_2dsaAnalysisControlGuiTest )

#include "test_us_2dsa_analysis_control_gui.moc"
