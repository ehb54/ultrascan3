//! \file test_us_timer.cpp
#include "qt_test_base.h"
#include "us_timer.h"
#include <QElapsedTimer>
#include <QRegularExpression>
#include <cmath>

// These tests check names, call counts, messages, ordering and the relation between the
// reported statistics, none of which depend on elapsed time. Exact durations, averages and
// standard deviations would need a controllable clock.

namespace
{
struct TimerLine
{
      QString    name;
      unsigned   count = 0;
      qulonglong total = 0;
      QString    avg;
      QString    sd;
};

::testing::AssertionResult parseLine( const QString& line, TimerLine& out )
{
   static const QRegularExpression re( "^(.*) (\\d+) calls total time (\\d+) ms avg time (\\S+) ms sd (\\S+) ms \\n$",
                                       QRegularExpression::DotMatchesEverythingOption );
   const auto                      m = re.match( line );
   if ( !m.hasMatch() )
   {
      return ::testing::AssertionFailure() << "unexpected list_time format: " << line.toStdString();
   }
   out.name  = m.captured( 1 );
   out.count = m.captured( 2 ).toUInt();
   out.total = m.captured( 3 ).toULongLong();
   out.avg   = m.captured( 4 );
   out.sd    = m.captured( 5 );
   return ::testing::AssertionSuccess();
}

QString undefinedMessage( const QString& name )
{
   return QString( "US_Timer::Error: undefined timer: %1\n" ).arg( name );
}

QString unusedMessage( const QString& name )
{
   return QString( "US_Timer::Error: no usage counts for timer: %1 \n" ).arg( name );
}

class US_TimerTest : public QtTestBase
{
   protected:
      US_Timer timer;

      void     cycle( const QString& name, int times = 1 )
      {
         for ( int i = 0; i < times; ++i )
         {
            ASSERT_TRUE( timer.start_timer( name ) );
            ASSERT_TRUE( timer.end_timer( name ) );
         }
      }

      // Name and call count only, for timers whose durations are undefined.
      void expectCalls( const QString& name, unsigned count )
      {
         TimerLine line;
         ASSERT_TRUE( parseLine( timer.list_time( name ), line ) );
         EXPECT_EQ( name, line.name );
         EXPECT_EQ( count, line.count );
      }

      // Name, count and the relation between total, average and standard deviation.
      void expectStats( const QString& name, unsigned count )
      {
         TimerLine line;
         ASSERT_TRUE( parseLine( timer.list_time( name ), line ) );
         EXPECT_EQ( name, line.name );
         EXPECT_EQ( count, line.count );
         EXPECT_EQ( QString( "%1" ).arg( static_cast<double>( line.total ) / count ), line.avg );
         bool         ok = false;
         const double sd = line.sd.toDouble( &ok );
         ASSERT_TRUE( ok ) << line.sd.toStdString();
         EXPECT_TRUE( std::isfinite( sd ) );
         EXPECT_GE( sd, 0.0 );
         if ( count == 1 )
         {
            EXPECT_EQ( QString( "0" ), line.sd );
         }
      }
};

using US_TimerEdgeCaseTest = US_TimerTest;
using US_TimerErrorTest    = US_TimerTest;
using US_TimerSequenceTest = US_TimerTest;
using US_TimerResourceTest = US_TimerTest;
using US_TimerOutputTest   = US_TimerTest;

// Constructor Tests
TEST_F( US_TimerTest, ConstructorInitializesCorrectly )
{
   EXPECT_TRUE( timer.list_times().isEmpty() );
   EXPECT_EQ( undefinedMessage( "anything" ), timer.list_time( "anything" ) );
}

TEST_F( US_TimerTest, ConstructorMultipleInstances )
{
   US_Timer timer2;
   timer.init_timer( "test1" );
   timer2.init_timer( "test2" );

   EXPECT_TRUE( timer.start_timer( "test1" ) );
   EXPECT_FALSE( timer2.start_timer( "test1" ) );
   EXPECT_EQ( unusedMessage( "test1" ), timer.list_times() );
   EXPECT_EQ( unusedMessage( "test2" ), timer2.list_times() );
}

// init_timer Tests
TEST_F( US_TimerTest, InitTimerValidName )
{
   timer.init_timer( "test_timer" );
   EXPECT_EQ( unusedMessage( "test_timer" ), timer.list_time( "test_timer" ) );
   EXPECT_TRUE( timer.start_timer( "test_timer" ) );
}

TEST_F( US_TimerTest, InitTimerEmptyString )
{
   timer.init_timer( "" );
   ASSERT_NO_FATAL_FAILURE( cycle( "" ) );
   expectStats( "", 1 );
}

TEST_F( US_TimerTest, InitTimerNullString )
{
   // A null and an empty QString name the same timer.
   timer.init_timer( QString() );
   EXPECT_TRUE( timer.start_timer( "" ) );
   EXPECT_EQ( unusedMessage( "" ), timer.list_times() );
}

TEST_F( US_TimerTest, InitTimerSpecialCharacters )
{
   const QString name = "timer@#$%^&*()_+{}|:<>?[];',./~`";
   timer.init_timer( name );
   ASSERT_NO_FATAL_FAILURE( cycle( name ) );
   expectStats( name, 1 );
}

TEST_F( US_TimerTest, InitTimerUnicodeCharacters )
{
   const QString name = QString::fromUtf8( "测试计时器_タイマー_🕐" );
   timer.init_timer( name );
   ASSERT_NO_FATAL_FAILURE( cycle( name ) );
   expectStats( name, 1 );
}

TEST_F( US_TimerTest, InitTimerVeryLongName )
{
   const QString name = QString( "timer" ).repeated( 1000 );
   timer.init_timer( name );
   ASSERT_NO_FATAL_FAILURE( cycle( name ) );
   expectStats( name, 1 );
}

TEST_F( US_TimerTest, InitTimerMultipleTimers )
{
   const QStringList names = { "timer3", "timer1", "timer5", "timer2", "timer4" };
   for ( const QString& name : names )
   {
      timer.init_timer( name );
   }
   EXPECT_EQ( unusedMessage( "timer1" ) + unusedMessage( "timer2" ) + unusedMessage( "timer3" ) + unusedMessage( "timer4" ) +
                 unusedMessage( "timer5" ),
              timer.list_times() );
}

TEST_F( US_TimerTest, InitTimerSameNameTwice )
{
   timer.init_timer( "duplicate_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "duplicate_timer", 2 ) );
   timer.init_timer( "duplicate_timer" );
   EXPECT_EQ( unusedMessage( "duplicate_timer" ), timer.list_time( "duplicate_timer" ) );

   ASSERT_NO_FATAL_FAILURE( cycle( "duplicate_timer" ) );
   expectCalls( "duplicate_timer", 1 );
}

// start_timer Tests
TEST_F( US_TimerTest, StartTimerValidTimer )
{
   timer.init_timer( "valid_timer" );
   EXPECT_TRUE( timer.start_timer( "valid_timer" ) );
   EXPECT_EQ( unusedMessage( "valid_timer" ), timer.list_time( "valid_timer" ) );
}

TEST_F( US_TimerTest, StartTimerNonExistentTimer )
{
   EXPECT_FALSE( timer.start_timer( "non_existent_timer" ) );
   EXPECT_TRUE( timer.list_times().isEmpty() );
}

TEST_F( US_TimerTest, StartTimerEmptyString )
{
   timer.init_timer( "" );
   EXPECT_TRUE( timer.start_timer( "" ) );
}

TEST_F( US_TimerTest, StartTimerMultipleTimes )
{
   timer.init_timer( "restart_timer" );
   EXPECT_TRUE( timer.start_timer( "restart_timer" ) );
   EXPECT_TRUE( timer.start_timer( "restart_timer" ) );
   EXPECT_TRUE( timer.start_timer( "restart_timer" ) );
   EXPECT_TRUE( timer.end_timer( "restart_timer" ) );
   expectStats( "restart_timer", 1 );
}

TEST_F( US_TimerTest, StartTimerAfterEnd )
{
   timer.init_timer( "end_start_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "end_start_timer", 2 ) );
   expectStats( "end_start_timer", 2 );
}

// end_timer Tests
TEST_F( US_TimerTest, EndTimerValidTimer )
{
   timer.init_timer( "valid_end_timer" );
   ASSERT_TRUE( timer.start_timer( "valid_end_timer" ) );
   EXPECT_TRUE( timer.end_timer( "valid_end_timer" ) );
   expectStats( "valid_end_timer", 1 );
}

TEST_F( US_TimerTest, EndTimerNonExistentTimer )
{
   EXPECT_FALSE( timer.end_timer( "non_existent_end_timer" ) );
   EXPECT_TRUE( timer.list_times().isEmpty() );
}

TEST_F( US_TimerTest, EndTimerWithoutStart )
{
   timer.init_timer( "end_without_start" );
   EXPECT_FALSE( timer.end_timer( "end_without_start" ) );
   EXPECT_EQ( unusedMessage( "end_without_start" ), timer.list_time( "end_without_start" ) );
}

TEST_F( US_TimerTest, EndTimerMultipleTimes )
{
   // Ending stops the timer, so only the first end after a start is recorded.
   timer.init_timer( "multiple_end_timer" );
   ASSERT_TRUE( timer.start_timer( "multiple_end_timer" ) );
   EXPECT_TRUE( timer.end_timer( "multiple_end_timer" ) );
   EXPECT_FALSE( timer.end_timer( "multiple_end_timer" ) );
   EXPECT_FALSE( timer.end_timer( "multiple_end_timer" ) );
   expectStats( "multiple_end_timer", 1 );
}

TEST_F( US_TimerTest, ReinitClearsPreviousStatistics )
{
   timer.init_timer( "reinit_timer" );
   ASSERT_TRUE( timer.start_timer( "reinit_timer" ) );
   QElapsedTimer spin;
   spin.start();
   while ( spin.elapsed() < 3 )
   {
   }
   ASSERT_TRUE( timer.end_timer( "reinit_timer" ) );

   timer.init_timer( "reinit_timer" );
   EXPECT_EQ( unusedMessage( "reinit_timer" ), timer.list_time( "reinit_timer" ) );
   ASSERT_NO_FATAL_FAILURE( cycle( "reinit_timer" ) );
   // One recorded call has no spread; a sum of squares kept from before the re-init would.
   TimerLine line;
   ASSERT_TRUE( parseLine( timer.list_time( "reinit_timer" ), line ) );
   EXPECT_EQ( 1u, line.count );
   EXPECT_EQ( QString( "0" ), line.sd );
}

TEST_F( US_TimerTest, EndTimerAccumulatesTime )
{
   timer.init_timer( "accumulate_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "accumulate_timer", 2 ) );
   expectStats( "accumulate_timer", 2 );
}

// list_time Tests
TEST_F( US_TimerTest, ListTimeValidTimer )
{
   timer.init_timer( "list_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "list_timer" ) );
   expectStats( "list_timer", 1 );
}

TEST_F( US_TimerTest, ListTimeNonExistentTimer )
{
   EXPECT_EQ( undefinedMessage( "non_existent_list_timer" ), timer.list_time( "non_existent_list_timer" ) );
}

TEST_F( US_TimerTest, ListTimeEmptyString )
{
   EXPECT_EQ( undefinedMessage( "" ), timer.list_time( "" ) );
}

TEST_F( US_TimerTest, ListTimeTimerWithoutUsage )
{
   timer.init_timer( "unused_timer" );
   EXPECT_EQ( unusedMessage( "unused_timer" ), timer.list_time( "unused_timer" ) );
}

TEST_F( US_TimerTest, ListTimeMultipleCalls )
{
   timer.init_timer( "multiple_calls_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "multiple_calls_timer", 5 ) );
   expectStats( "multiple_calls_timer", 5 );
}

TEST_F( US_TimerTest, ListTimeZeroTimeElapsed )
{
   timer.init_timer( "zero_time_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "zero_time_timer" ) );
   expectStats( "zero_time_timer", 1 );
}

TEST_F( US_TimerTest, ListTimeStatisticalCalculations )
{
   timer.init_timer( "stats_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "stats_timer", 3 ) );
   expectStats( "stats_timer", 3 );
}

// list_times Tests
TEST_F( US_TimerTest, ListTimesEmptyTimer )
{
   EXPECT_TRUE( timer.list_times().isEmpty() );
}

TEST_F( US_TimerTest, ListTimesSingleTimer )
{
   timer.init_timer( "single_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "single_timer" ) );
   EXPECT_EQ( timer.list_time( "single_timer" ), timer.list_times() );
   expectStats( "single_timer", 1 );
}

TEST_F( US_TimerTest, ListTimesMultipleTimers )
{
   for ( const QString& name : QStringList{ "timer_c", "timer_a", "timer_b" } )
   {
      timer.init_timer( name );
      ASSERT_NO_FATAL_FAILURE( cycle( name ) );
   }
   EXPECT_EQ( timer.list_time( "timer_a" ) + timer.list_time( "timer_b" ) + timer.list_time( "timer_c" ), timer.list_times() );
}

TEST_F( US_TimerTest, ListTimesWithUnusedTimers )
{
   timer.init_timer( "used_timer" );
   timer.init_timer( "unused_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "used_timer" ) );

   EXPECT_EQ( unusedMessage( "unused_timer" ) + timer.list_time( "used_timer" ), timer.list_times() );
   expectStats( "used_timer", 1 );
}

TEST_F( US_TimerTest, ListTimesWithMixedTimers )
{
   timer.init_timer( "good_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "good_timer" ) );
   timer.init_timer( "bad_timer" );

   EXPECT_EQ( unusedMessage( "bad_timer" ) + timer.list_time( "good_timer" ), timer.list_times() );
   expectStats( "good_timer", 1 );
}

// Edge Case Tests
TEST_F( US_TimerEdgeCaseTest, VeryShortTimeMeasurement )
{
   timer.init_timer( "micro_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "micro_timer" ) );
   expectStats( "micro_timer", 1 );
}

TEST_F( US_TimerEdgeCaseTest, LargeNumberOfCalls )
{
   timer.init_timer( "many_calls_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "many_calls_timer", 1000 ) );
   expectStats( "many_calls_timer", 1000 );
}

TEST_F( US_TimerEdgeCaseTest, TimerNameWithWhitespace )
{
   const QString name = "  timer with spaces  ";
   timer.init_timer( name );
   ASSERT_NO_FATAL_FAILURE( cycle( name ) );
   expectStats( name, 1 );
}

TEST_F( US_TimerEdgeCaseTest, TimerNameWithNewlines )
{
   const QString name = "timer\nwith\nnewlines";
   timer.init_timer( name );
   ASSERT_NO_FATAL_FAILURE( cycle( name ) );
   expectStats( name, 1 );
}

TEST_F( US_TimerEdgeCaseTest, TimerNameWithTabsAndCarriageReturns )
{
   const QString name = "timer\twith\tspecial\rchars";
   timer.init_timer( name );
   ASSERT_NO_FATAL_FAILURE( cycle( name ) );
   expectStats( name, 1 );
}

TEST_F( US_TimerEdgeCaseTest, CaseSensitiveTimerNames )
{
   timer.init_timer( "Timer" );
   timer.init_timer( "timer" );
   timer.init_timer( "TIMER" );

   EXPECT_TRUE( timer.start_timer( "Timer" ) );
   EXPECT_TRUE( timer.start_timer( "timer" ) );
   EXPECT_TRUE( timer.start_timer( "TIMER" ) );
   EXPECT_FALSE( timer.start_timer( "TiMeR" ) );
   EXPECT_EQ( unusedMessage( "TIMER" ) + unusedMessage( "Timer" ) + unusedMessage( "timer" ), timer.list_times() );
}

// Error Condition Tests
TEST_F( US_TimerErrorTest, StartNonExistentTimer )
{
   EXPECT_FALSE( timer.start_timer( "does_not_exist" ) );
}

TEST_F( US_TimerErrorTest, EndNonExistentTimer )
{
   EXPECT_FALSE( timer.end_timer( "does_not_exist" ) );
}

TEST_F( US_TimerErrorTest, ListNonExistentTimer )
{
   EXPECT_EQ( undefinedMessage( "does_not_exist" ), timer.list_time( "does_not_exist" ) );
}

TEST_F( US_TimerErrorTest, MultipleEndWithoutStart )
{
   timer.init_timer( "end_without_start" );
   EXPECT_FALSE( timer.end_timer( "end_without_start" ) );
   EXPECT_FALSE( timer.end_timer( "end_without_start" ) );
   EXPECT_FALSE( timer.end_timer( "end_without_start" ) );
   EXPECT_EQ( unusedMessage( "end_without_start" ), timer.list_time( "end_without_start" ) );
}

TEST_F( US_TimerErrorTest, StartAfterMultipleEnds )
{
   timer.init_timer( "start_after_ends" );
   EXPECT_FALSE( timer.end_timer( "start_after_ends" ) );
   EXPECT_FALSE( timer.end_timer( "start_after_ends" ) );
   EXPECT_TRUE( timer.start_timer( "start_after_ends" ) );
   EXPECT_TRUE( timer.end_timer( "start_after_ends" ) );
   expectStats( "start_after_ends", 1 );
}

// Method Sequence Tests
TEST_F( US_TimerSequenceTest, CorrectSequenceInitStartEnd )
{
   timer.init_timer( "correct_sequence" );
   EXPECT_TRUE( timer.start_timer( "correct_sequence" ) );
   EXPECT_TRUE( timer.end_timer( "correct_sequence" ) );
   expectStats( "correct_sequence", 1 );
}

TEST_F( US_TimerSequenceTest, StartEndMultipleCycles )
{
   timer.init_timer( "cycle_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "cycle_timer", 10 ) );
   expectStats( "cycle_timer", 10 );
}

TEST_F( US_TimerSequenceTest, OverlappingTimers )
{
   timer.init_timer( "timer1" );
   timer.init_timer( "timer2" );

   ASSERT_TRUE( timer.start_timer( "timer1" ) );
   ASSERT_TRUE( timer.start_timer( "timer2" ) );
   ASSERT_TRUE( timer.end_timer( "timer1" ) );
   EXPECT_FALSE( timer.end_timer( "timer1" ) );
   ASSERT_TRUE( timer.end_timer( "timer2" ) );

   expectStats( "timer1", 1 );
   expectStats( "timer2", 1 );
}

// Memory and Resource Tests
TEST_F( US_TimerResourceTest, MultipleTimerInstances )
{
   std::vector<std::unique_ptr<US_Timer>> timers;
   for ( int i = 0; i < 10; ++i )
   {
      timers.push_back( std::make_unique<US_Timer>() );
      const QString name = QString( "timer_%1" ).arg( i );
      timers[i]->init_timer( name );
      timers[i]->start_timer( name );
      timers[i]->end_timer( name );
   }

   for ( int i = 0; i < 10; ++i )
   {
      const QString name = QString( "timer_%1" ).arg( i );
      TimerLine     line;
      ASSERT_TRUE( parseLine( timers[i]->list_times(), line ) );
      EXPECT_EQ( name, line.name );
      EXPECT_EQ( 1u, line.count );
   }
}

// Format and Output Tests
TEST_F( US_TimerOutputTest, OutputFormatConsistency )
{
   timer.init_timer( "format_timer" );
   ASSERT_NO_FATAL_FAILURE( cycle( "format_timer" ) );
   TimerLine line;
   EXPECT_TRUE( parseLine( timer.list_time( "format_timer" ), line ) );
}

TEST_F( US_TimerOutputTest, ErrorMessageFormat )
{
   EXPECT_EQ( QString( "US_Timer::Error: undefined timer: undefined\n" ), timer.list_time( "undefined" ) );
   timer.init_timer( "idle" );
   EXPECT_EQ( QString( "US_Timer::Error: no usage counts for timer: idle \n" ), timer.list_time( "idle" ) );
}

TEST_F( US_TimerOutputTest, StatisticsAreInternallyConsistent )
{
   timer.init_timer( "stats_consistency" );
   ASSERT_NO_FATAL_FAILURE( cycle( "stats_consistency", 7 ) );
   expectStats( "stats_consistency", 7 );
}
} // namespace
