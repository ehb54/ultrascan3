//! \file test_us_autoflow_profile_refusal_gui.cpp
#include "us3_gui_test_main.h"
#include "us_autoflow_analysis.h"

#include <QSignalSpy>
#include <QTimer>

class US_AutoflowProfileRefusalTest : public QObject
{
      Q_OBJECT

   private slots:
      void resetsBeforeReturningToRunManager_data()
      {
         QTest::addColumn<bool>( "initializing" );
         QTest::newRow( "ABDE setup" ) << true;
         QTest::newRow( "VEL-MWL 2DSA" ) << false;
      }

      void resetsBeforeReturningToRunManager()
      {
         QFETCH( bool, initializing );

         US_Analysis_auto analysis;
         QSignalSpy       close_setup( &analysis, &US_Analysis_auto::close_analysissetup_msg );
         QSignalSpy       back_to_runs( &analysis, &US_Analysis_auto::analysis_back_to_initAutoflow );
         analysis.timer_update->start( 5000 );
         US_GmpProgress::acquire( &analysis, analysis.progress_msg_mwlsim )->show();
         US_GmpProgress::acquire( &analysis, analysis.progress_msg_2dsa )->show();
         analysis.twodsa_profile_ready = true;
         analysis.twodsa_chan_idx      = -1;
         analysis.channels_2dsa_approved << "1 / A";
         analysis.channels_2dsa_filenames["1 / A"]              = "run";
         analysis.protocol_details_at_analysis_2dsa["filename"] = "run";

         connect( &analysis,
                  &US_Analysis_auto::analysis_back_to_initAutoflow,
                  this,
                  [&]()
                  {
                     QVERIFY( !analysis.timer_update->isActive() );
                     QVERIFY( !analysis.progress_msg_mwlsim->isVisible() );
                     QVERIFY( !analysis.progress_msg_2dsa->isVisible() );
                     QVERIFY( analysis.channels_2dsa_approved.isEmpty() );
                     QVERIFY( analysis.channels_2dsa_filenames.isEmpty() );
                     QVERIFY( analysis.protocol_details_at_analysis_2dsa.isEmpty() );
                     QCOMPARE( close_setup.count(), initializing ? 1 : 0 );
                  } );

         analysis.abort_profile_setup( initializing );
         QCOMPARE( back_to_runs.count(), 1 );

         // A callback queued by the previous channel cannot advance the pipeline.
         analysis.start_next_2dsa_channel();
         QCOMPARE( analysis.twodsa_chan_idx, -1 );
      }
};

US3_GUI_TEST_MAIN( US_AutoflowProfileRefusalTest )
#include "test_us_autoflow_profile_refusal_gui.moc"
