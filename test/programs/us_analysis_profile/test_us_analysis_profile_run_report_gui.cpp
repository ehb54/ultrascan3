//! \file test_us_analysis_profile_run_report_gui.cpp
//! Loading profiles whose Run or Report boxes are off, and clicking the boxes by hand.
#include "run_report_clicks.h"
#include "us3_gui_test_main.h"
#include "us_analysis_profile.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>

namespace
{
const int NCHAN = 3;

//! The values of the General tab that the Run and Report boxes change
struct Values
{
      QList<int> run;
      QList<int> report;
      QList<int> ref;
      QList<int> use;

      bool       operator==( const Values& other ) const
      {
         return run == other.run && report == other.report && ref == other.ref && use == other.use;
      }
};

QString text( const QList<int>& list )
{
   QStringList parts;
   for ( int value : list )
   {
      parts << QString::number( value );
   }
   return parts.join( "," );
}

QString text( const Values& values )
{
   return QString( "run %1 report %2 ref %3 use %4" )
      .arg( text( values.run ), text( values.report ), text( values.ref ), text( values.use ) );
}

US_RunProtocol make_protocol()
{
   US_RunProtocol protocol;
   protocol.protoname         = "Run and Report";
   protocol.protoGUID         = "13360000-0000-0000-0000-000000000001";
   protocol.rpAprof.aprofname = protocol.protoname;
   protocol.rpSolut.chsols.clear();
   protocol.rpOptic.chopts.clear();
   protocol.rpRange.chrngs.clear();
   const QStringList channels = { "1 / A", "1 / B", "2 / A" };
   for ( const QString& channel : channels )
   {
      US_RunProtocol::RunProtoSolutions::ChanSolu solution;
      solution.channel  = channel;
      solution.solution = "Sample";
      protocol.rpSolut.chsols << solution;

      US_RunProtocol::RunProtoOptics::OpticSys optics;
      optics.channel = channel;
      optics.scan1   = "UV/visible";
      protocol.rpOptic.chopts << optics;

      US_RunProtocol::RunProtoRanges::Ranges range;
      range.channel = channel;
      range.wvlens  = { 280.0 };
      protocol.rpRange.chrngs << range;
   }
   return protocol;
}

//! A profile window with one loaded protocol
class Window
{
   public:
      explicit Window( bool abde )
      {
         protocol                  = make_protocol();
         window.currProf.aprofname = protocol.protoname;
         window.abde_mode_aprofile = abde;
      }

      void load( const US_AnaProfile& profile )
      {
         window.currProf = profile;
         window.inherit_protocol( &protocol );
      }

      void load()
      {
         window.inherit_protocol( &protocol );
      }

      //! Leave the General tab, which saves it, and return to it, which loads it again
      Values save()
      {
         auto* tabs = window.findChild<QTabWidget*>();
         tabs->setCurrentIndex( 1 );
         const Values saved = values();
         tabs->setCurrentIndex( 0 );
         return saved;
      }

      Values values() const
      {
         const US_AnaProfile& profile = window.currProf;
         return { profile.analysis_run, profile.report_run, profile.ref_channels, profile.ref_use_channels };
      }

      //! Click a box, given as a name such as "1: Run"
      bool click( const QString& name )
      {
         auto* box = window.findChild<QCheckBox*>( name );
         if ( box == nullptr || !box->isEnabled() )
         {
            return false;
         }
         box->click();
         return true;
      }

      US_RunProtocol        protocol;
      US_AnalysisProfileGui window;
};
}

class US_AnalysisProfileRunReportTest : public QObject
{
      Q_OBJECT

   private slots:
      void initTestCase()
      {
         US_Settings::set_default_data_location( 2 );
         QVERIFY( QDir().mkpath( US_Settings::dataDir() + "/projects" ) );
      }

      void load_keeps_saved_values_data();
      void load_keeps_saved_values();
      void clicking_by_hand_data();
      void clicking_by_hand();
      void clicking_after_load_data();
      void clicking_after_load();
      void clicks_keep_a_chosen_reference();
      void panels_have_the_loaded_channels();
      void report_on_without_a_reference();
      void unusable_profile_keeps_the_one_shown();
};

void US_AnalysisProfileRunReportTest::load_keeps_saved_values_data()
{
   QTest::addColumn<bool>( "abde" );
   QTest::addColumn<QList<int>>( "run" );
   QTest::addColumn<QList<int>>( "report" );
   QTest::addColumn<QList<int>>( "ref" );
   QTest::addColumn<QList<int>>( "use" );

   for ( bool abde : { false, true } )
   {
      const QString mode = abde ? "abde " : "velocity ";
      for ( int off = 0; off < NCHAN; ++off )
      {
         QList<int> on;
         QList<int> none;
         for ( int ii = 0; ii < NCHAN; ++ii )
         {
            on << ( ii == off ? 0 : 1 );
            none << 0;
         }
         QList<int> all  = { 1, 1, 1 };
         QList<int> ref  = none;
         ref[off]        = 1;
         const QString n = QString::number( off );
         QTest::newRow( qPrintable( mode + "report off " + n ) ) << abde << all << on << ref << on;
         QTest::newRow( qPrintable( mode + "run off " + n ) ) << abde << on << on << ref << on;
         // As in a profile saved before the reference attributes existed
         QTest::newRow( qPrintable( mode + "report off, no reference " + n ) ) << abde << all << on << none << none;
         QTest::newRow( qPrintable( mode + "run off, no reference " + n ) ) << abde << on << on << none << none;
         // The use-reference exceeds half of the channels
         QList<int> one_on      = none;
         QList<int> refs        = { 1, 1, 1 };
         QList<int> use_two     = none;
         one_on[off]            = 1;
         refs[off]              = 0;
         refs[off == 2 ? 1 : 2] = 2;
         use_two[off]           = 2;
         QTest::newRow( qPrintable( mode + "two references for " + n ) ) << abde << one_on << one_on << refs << use_two;
      }
      QTest::newRow( qPrintable( mode + "all off" ) )
         << abde << QList<int>{ 0, 0, 0 } << QList<int>{ 0, 0, 0 } << QList<int>{ 1, 2, 3 } << QList<int>{ 0, 0, 0 };
      QTest::newRow( qPrintable( mode + "run off and report on" ) )
         << abde << QList<int>{ 1, 0, 1 } << QList<int>{ 1, 1, 0 } << QList<int>{ 0, 0, 1 } << QList<int>{ 1, 0, 0 };
      QTest::newRow( qPrintable( mode + "two references, the first one used" ) )
         << abde << QList<int>{ 1, 1, 1 } << QList<int>{ 0, 1, 0 } << QList<int>{ 1, 0, 2 } << QList<int>{ 0, 1, 0 };
   }
   QTest::newRow( "velocity all on" ) << false << QList<int>{ 1, 1, 1 } << QList<int>{ 1, 1, 1 } << QList<int>{ 0, 0, 0 }
                                      << QList<int>{ 0, 0, 0 };
   QTest::newRow( "velocity run off and all reports on" )
      << false << QList<int>{ 1, 0, 1 } << QList<int>{ 1, 1, 1 } << QList<int>{ 0, 0, 0 } << QList<int>{ 0, 0, 0 };
}

void US_AnalysisProfileRunReportTest::load_keeps_saved_values()
{
   QFETCH( bool, abde );
   QFETCH( QList<int>, run );
   QFETCH( QList<int>, report );
   QFETCH( QList<int>, ref );
   QFETCH( QList<int>, use );
   const Values expected{ run, report, ref, use };

   Window       plain( abde );
   plain.load();
   US_AnaProfile saved    = plain.window.currProf;
   saved.analysis_run     = run;
   saved.report_run       = report;
   saved.ref_channels     = ref;
   saved.ref_use_channels = use;

   Window loaded( abde );
   loaded.load( saved );
   QCOMPARE( text( loaded.values() ), text( expected ) );
   QCOMPARE( text( loaded.save() ), text( expected ) );
   QCOMPARE( text( loaded.values() ), text( expected ) );

   // Again in the same window
   loaded.load();
   QCOMPARE( text( loaded.values() ), text( expected ) );
   QCOMPARE( text( loaded.save() ), text( expected ) );
}

void US_AnalysisProfileRunReportTest::clicking_by_hand_data()
{
   QTest::addColumn<bool>( "abde" );
   QTest::addColumn<QStringList>( "clicks" );
   QTest::addColumn<QString>( "expected" );

   for ( const RunReportClicks& row : run_report_clicks() )
   {
      const QString mode = row.abde ? "abde " : "velocity ";
      QTest::newRow( qPrintable( mode + row.clicks.join( ", " ) ) ) << row.abde << row.clicks << row.expected;
   }
}

void US_AnalysisProfileRunReportTest::clicking_by_hand()
{
   QFETCH( bool, abde );
   QFETCH( QStringList, clicks );
   QFETCH( QString, expected );
   Window window( abde );
   window.load();
   for ( const QString& name : clicks )
   {
      QVERIFY2( window.click( name ), qPrintable( name ) );
   }
   // The profile is saved on leaving the tab, before any reload
   window.window.findChild<QTabWidget*>()->setCurrentIndex( 1 );
   QCOMPARE( text( window.values() ), expected );
}

void US_AnalysisProfileRunReportTest::clicking_after_load_data()
{
   QTest::addColumn<bool>( "abde" );
   QTest::addColumn<QString>( "first" );
   QTest::addColumn<QString>( "second" );

   for ( bool abde : { false, true } )
   {
      for ( int ii = 0; ii < NCHAN; ++ii )
      {
         // With every report on, an ABDE load clears the first B channel's Run box again
         if ( abde && ii == 1 )
         {
            continue;
         }
         for ( int jj = 0; jj < NCHAN; ++jj )
         {
            for ( const char* box : { ": Run", ": RunReport" } )
            {
               const QString first  = QString::number( ii ) + box;
               const QString second = QString::number( jj ) + box;
               QTest::newRow( qPrintable( QString( "%1 %2, %3" ).arg( abde ? "abde" : "velocity", first, second ) ) )
                  << abde << first << second;
            }
         }
      }
   }
}

//! Clicks, a save, a load in a new window and another click give what the clicks alone give
void US_AnalysisProfileRunReportTest::clicking_after_load()
{
   QFETCH( bool, abde );
   QFETCH( QString, first );
   QFETCH( QString, second );

   Window by_hand( abde );
   by_hand.load();
   QVERIFY( by_hand.click( first ) );
   Window saved( abde );
   saved.load();
   QVERIFY( saved.click( first ) );
   saved.window.findChild<QTabWidget*>()->setCurrentIndex( 1 );

   Window loaded( abde );
   loaded.load( saved.window.currProf );
   QCOMPARE( text( loaded.values() ), text( saved.values() ) );

   const bool clicked = by_hand.click( second );
   QCOMPARE( loaded.click( second ), clicked );
   by_hand.window.findChild<QTabWidget*>()->setCurrentIndex( 1 );
   loaded.window.findChild<QTabWidget*>()->setCurrentIndex( 1 );
   QCOMPARE( text( loaded.values() ), text( by_hand.values() ) );
}

//! A use-reference that was just chosen is not lost when a box is clicked
void US_AnalysisProfileRunReportTest::clicks_keep_a_chosen_reference()
{
   Window plain( true );
   plain.load();
   US_AnaProfile saved    = plain.window.currProf;
   saved.analysis_run     = { 1, 1, 1 };
   saved.report_run       = { 1, 0, 0 };
   saved.ref_channels     = { 0, 1, 2 };
   saved.ref_use_channels = { 2, 0, 0 };

   Window loaded( true );
   loaded.load( saved );
   auto* use = loaded.window.findChild<QSpinBox*>( "0: RefUseChan --chann_name--1A:UV/vis.:Sample" );
   QVERIFY( use != nullptr );
   use->setValue( 1 );
   QVERIFY( loaded.click( "1: Run" ) );
   loaded.window.findChild<QTabWidget*>()->setCurrentIndex( 1 );
   QCOMPARE( text( loaded.values() ), QString( "run 1,0,1 report 1,0,0 ref 0,1,2 use 1,0,0" ) );
}

void US_AnalysisProfileRunReportTest::panels_have_the_loaded_channels()
{
   Window            window( false );
   const QStringList channels = { "1A:UV/vis.:Sample", "1B:UV/vis.:Sample", "2A:UV/vis.:Sample" };
   window.load();
   for ( const char* panel : { "general", "2dsa", "pcsa" } )
   {
      QCOMPARE( window.window.childLValue( panel, "channels" ), channels );
   }
}

//! A channel whose Report is off can have no reference number, as in older profiles
void US_AnalysisProfileRunReportTest::report_on_without_a_reference()
{
   for ( bool abde : { false, true } )
   {
      Window plain( abde );
      plain.load();
      US_AnaProfile saved    = plain.window.currProf;
      saved.analysis_run     = { 1, 1, 1 };
      saved.report_run       = { 0, 1, 1 };
      saved.ref_channels     = { 0, 0, 0 };
      saved.ref_use_channels = { 0, 0, 0 };

      Window loaded( abde );
      loaded.load( saved );
      QVERIFY( loaded.click( "0: RunReport" ) );
      loaded.window.findChild<QTabWidget*>()->setCurrentIndex( 1 );
      QCOMPARE( text( loaded.values() ), QString( "run 1,1,1 report 1,1,1 ref 0,0,0 use 0,0,0" ) );
   }
}

void US_AnalysisProfileRunReportTest::unusable_profile_keeps_the_one_shown()
{
   Window window( false );
   window.load();
   const US_AnaProfile before = window.window.currProf;

   // An optic that is not installed comes before a usable one
   window.protocol.rpOptic.chopts[0].scan1 = "(not installed)";
   window.protocol.rpOptic.chopts[0].scan2 = "UV/visible";
   QStringList shown;
   QTimer      close;
   connect( &close,
            &QTimer::timeout,
            [&]
            {
               auto* message = qobject_cast<QMessageBox*>( QApplication::activeModalWidget() );
               if ( message != nullptr )
               {
                  shown << message->text();
                  message->accept();
               }
            } );
   close.start( 10 );
   window.load();
   QCOMPARE( shown.size(), 1 );
   QVERIFY( window.window.currProf == before );
   QCOMPARE( window.window.currProf.chndescs, before.chndescs );
}

US3_GUI_TEST_MAIN( US_AnalysisProfileRunReportTest )
#include "test_us_analysis_profile_run_report_gui.moc"
