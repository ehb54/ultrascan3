#include "us3_gui_test_main.h"

#include "us_astfem_sim.h"
#include "us_math2.h"

namespace
{
US_DataIO::RawData compositeData()
{
   US_DataIO::RawData data;
   data.xvalues << 5.8 << 5.9 << 6.0;
   data.scanData.resize( 3 );

   for ( int scan = 0; scan < data.scanData.size(); scan++ )
   {
      data.scanData[ scan ].seconds = ( scan + 1 ) * 100.0;
      data.scanData[ scan ].rvalues.fill( scan + 1.0, 3 );
   }

   return data;
}

}

class US_AstfemSimNoiseTest : public QObject
{
   Q_OBJECT

   static void configureTwoSpeeds( US_Astfem_Sim& sim )
   {
      sim.simparams.speed_step.resize( 2 );
      sim.simparams.speed_step[ 0 ].scans = 1;
      sim.simparams.speed_step[ 1 ].scans = 2;

      sim.sim_data_all = compositeData();
      sim.sim_datas.resize( 2 );
      sim.sim_datas[ 0 ].xvalues = sim.sim_data_all.xvalues;
      sim.sim_datas[ 1 ].xvalues = sim.sim_data_all.xvalues;
      sim.sim_datas[ 0 ].scanData.resize( 1 );
      sim.sim_datas[ 1 ].scanData.resize( 2 );
   }

   static void seedSpeedViewsFromComposite( US_Astfem_Sim& sim )
   {
      int scan_offset = 0;

      for ( US_DataIO::RawData& speed_data : sim.sim_datas )
      {
         for ( int scan = 0; scan < speed_data.scanData.size(); scan++ )
            speed_data.scanData[ scan ] =
               sim.sim_data_all.scanData[ scan_offset + scan ];

         scan_offset += speed_data.scanData.size();
      }
   }

   static void applyNoise( US_Astfem_Sim& sim )
   {
      sim.ri_noise();
      sim.baseline();
      sim.random_noise();
      sim.ti_noise();
   }

private slots:
   void canonicalNoiseMatchesLegacyPerSpeedTraversal_data()
   {
      QTest::addColumn< double >( "ri" );
      QTest::addColumn< double >( "baseline" );
      QTest::addColumn< double >( "random" );
      QTest::addColumn< double >( "local" );
      QTest::addColumn< double >( "ti" );

      QTest::newRow( "radially invariant" ) << 1.2 << 0.0 << 0.0 << 0.0 << 0.0;
      QTest::newRow( "baseline" )           << 0.0 << 0.3 << 0.0 << 0.0 << 0.0;
      QTest::newRow( "random" )             << 0.0 << 0.0 << 1.4 << 0.0 << 0.0;
      QTest::newRow( "local random" )       << 0.0 << 0.0 << 0.0 << 1.6 << 0.0;
      QTest::newRow( "time invariant" )     << 0.0 << 0.0 << 0.0 << 0.0 << 0.8;
      QTest::newRow( "all components" )     << 1.2 << 0.3 << 1.4 << 1.6 << 0.8;
   }

   void canonicalNoiseMatchesLegacyPerSpeedTraversal()
   {
      QFETCH( double, ri );
      QFETCH( double, baseline );
      QFETCH( double, random );
      QFETCH( double, local );
      QFETCH( double, ti );

      US_Astfem_Sim canonical;
      US_Astfem_Sim legacy;
      configureTwoSpeeds( canonical );
      configureTwoSpeeds( legacy );
      seedSpeedViewsFromComposite( legacy );

      canonical.total_conc = legacy.total_conc = 2.0;
      canonical.simparams.rinoise = legacy.simparams.rinoise = ri;
      canonical.simparams.baseline = legacy.simparams.baseline = baseline;
      canonical.simparams.rnoise = legacy.simparams.rnoise = random;
      canonical.simparams.lrnoise = legacy.simparams.lrnoise = local;
      canonical.simparams.tinoise = legacy.simparams.tinoise = ti;
      canonical.noise_to_composite = true;
      legacy.noise_to_composite = false;

      US_Math2::randomize( 0x5eed1234U );
      applyNoise( canonical );
      canonical.derive_speed_data();

      US_Math2::randomize( 0x5eed1234U );
      applyNoise( legacy );

      QCOMPARE( canonical.sim_datas.size(), legacy.sim_datas.size() );
      for ( int speed = 0; speed < canonical.sim_datas.size(); speed++ )
      {
         QCOMPARE( canonical.sim_datas[ speed ].scanData.size(),
                   legacy.sim_datas[ speed ].scanData.size() );

         for ( int scan = 0;
               scan < canonical.sim_datas[ speed ].scanData.size(); scan++ )
            QCOMPARE( canonical.sim_datas[ speed ].scanData[ scan ].rvalues,
                      legacy.sim_datas[ speed ].scanData[ scan ].rvalues );
      }

      QCOMPARE( canonical.csv_data_ri.size(), legacy.csv_data_ri.size() );
   }

   void baselineFinalizesTheCompositeBeforeDerivingSpeedViews()
   {
      US_Astfem_Sim sim;
      configureTwoSpeeds( sim );
      sim.simparams.baseline = 0.25;

      sim.baseline();
      sim.derive_speed_data();

      for ( int scan = 0; scan < sim.sim_data_all.scanData.size(); scan++ )
      {
         for ( double value : sim.sim_data_all.scanData[ scan ].rvalues )
            QCOMPARE( value, scan + 1.25 );
      }

      QCOMPARE( sim.sim_datas[ 0 ].scanData[ 0 ].rvalues,
                sim.sim_data_all.scanData[ 0 ].rvalues );
      QCOMPARE( sim.sim_datas[ 1 ].scanData[ 0 ].rvalues,
                sim.sim_data_all.scanData[ 1 ].rvalues );
      QCOMPARE( sim.sim_datas[ 1 ].scanData[ 1 ].rvalues,
                sim.sim_data_all.scanData[ 2 ].rvalues );
   }

   void riNoiseCoversEveryScanInTheCompositeAndCsv()
   {
      US_Astfem_Sim sim;
      configureTwoSpeeds( sim );
      sim.total_conc       = 1.0;
      sim.simparams.rinoise = 1.0;

      const US_DataIO::RawData before = sim.sim_data_all;
      sim.ri_noise();
      sim.derive_speed_data();

      // One CSV per speed step, each covering that step's scans, because each
      // step is saved as its own run beside its own noise record.
      QCOMPARE( sim.csv_data_ri.size(), 2 );
      QCOMPARE( sim.csv_data_ri[ 0 ].rowCount(), 1 );
      QCOMPARE( sim.csv_data_ri[ 1 ].rowCount(), 2 );

      for ( int scan = 0; scan < sim.sim_data_all.scanData.size(); scan++ )
      {
         const QVector<double>& values =
            sim.sim_data_all.scanData[ scan ].rvalues;
         const double offset = values[ 0 ]
                             - before.scanData[ scan ].rvalues[ 0 ];

         for ( int point = 1; point < values.size(); point++ )
            QCOMPARE( values[ point ]
                    - before.scanData[ scan ].rvalues[ point ], offset );
      }

      QCOMPARE( sim.sim_datas[ 1 ].scanData[ 1 ].rvalues,
                sim.sim_data_all.scanData[ 2 ].rvalues );
   }

   void astfvmNoiseStaysOnItsOwnPerSpeedDatasets()
   {
      // ASTFVM fills only sim_datas[ 0 ] and leaves the composite empty, so
      // the canonical-composite traversal must not be used there: indexing it
      // by an offset accumulated across every speed step would run off the
      // end.  Its historical per-speed behavior is preserved as found.
      US_Astfem_Sim sim;
      configureTwoSpeeds( sim );
      sim.sim_data_all.scanData.clear();
      sim.noise_to_composite = false;
      sim.simparams.baseline = 0.25;

      for ( int step = 0; step < sim.sim_datas.size(); step++ )
         for ( US_DataIO::Scan& scan : sim.sim_datas[ step ].scanData )
            scan.rvalues.fill( 0.0, 3 );

      sim.baseline();

      for ( int step = 0; step < sim.sim_datas.size(); step++ )
         for ( const US_DataIO::Scan& scan : sim.sim_datas[ step ].scanData )
            for ( double value : scan.rvalues )
               QCOMPARE( value, 0.25 );
   }
};

US3_GUI_TEST_MAIN(US_AstfemSimNoiseTest)

#include "test_us_astfem_noise_gui.moc"
