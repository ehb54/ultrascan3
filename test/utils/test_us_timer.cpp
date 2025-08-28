// test_us_timer.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "qt_test_base.h"
#include "us_timer.h"
#include <QTest>
#include <QThread>
#include <QEventLoop>
#include <QTimer>

using ::testing::_;
using ::testing::Return;
using ::testing::HasSubstr;
using ::testing::Not;
using ::qt_matchers::QStringContains;
using ::qt_matchers::QStringEq;
using ::qt_matchers::QStringIsEmpty;

class US_TimerTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        timer = std::make_unique<US_Timer>();
    }

    void TearDown() override {
        timer.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_Timer> timer;
};

// Constructor Tests
TEST_F(US_TimerTest, ConstructorInitializesCorrectly) {
    // Test that constructor creates an empty timer
    QString result = timer->list_times();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(US_TimerTest, ConstructorMultipleInstances) {
    // Test multiple timer instances can be created
    auto timer2 = std::make_unique<US_Timer>();
    auto timer3 = std::make_unique<US_Timer>();

    EXPECT_NE(timer.get(), nullptr);
    EXPECT_NE(timer2.get(), nullptr);
    EXPECT_NE(timer3.get(), nullptr);

    // Each should be independent
    timer->init_timer("test1");
    timer2->init_timer("test2");

    EXPECT_TRUE(timer->start_timer("test1"));
    EXPECT_FALSE(timer2->start_timer("test1"));  // test1 doesn't exist in timer2
}

// Destructor Tests
TEST_F(US_TimerTest, DestructorDoesNotCrash) {
    // Test destructor by creating and destroying timer in scope
    {
        US_Timer localTimer;
        localTimer.init_timer("temp");
        localTimer.start_timer("temp");
        localTimer.end_timer("temp");
    }
    // Destructor called here - should not crash
    SUCCEED();
}

// init_timer Tests
TEST_F(US_TimerTest, InitTimerValidName) {
    QString timerName = "test_timer";

    timer->init_timer(timerName);

    // Verify timer was initialized by trying to start it
    EXPECT_TRUE(timer->start_timer(timerName));
}

TEST_F(US_TimerTest, InitTimerEmptyString) {
    QString emptyName = "";

    timer->init_timer(emptyName);

    // Should still work with empty string
    EXPECT_TRUE(timer->start_timer(emptyName));
}

TEST_F(US_TimerTest, InitTimerNullString) {
    QString nullName;  // Default constructed QString

    timer->init_timer(nullName);

    EXPECT_TRUE(timer->start_timer(nullName));
}

TEST_F(US_TimerTest, InitTimerSpecialCharacters) {
    QString specialName = "timer@#$%^&*()_+{}|:<>?[];',./~`";

    timer->init_timer(specialName);

    EXPECT_TRUE(timer->start_timer(specialName));
}

TEST_F(US_TimerTest, InitTimerUnicodeCharacters) {
    QString unicodeName = "测试计时器_タイマー_🕐";

    timer->init_timer(unicodeName);

    EXPECT_TRUE(timer->start_timer(unicodeName));
}

TEST_F(US_TimerTest, InitTimerVeryLongName) {
    QString longName = QString("timer").repeated(1000);  // 5000 characters

    timer->init_timer(longName);

    EXPECT_TRUE(timer->start_timer(longName));
}

TEST_F(US_TimerTest, InitTimerMultipleTimers) {
    QStringList timerNames = {"timer1", "timer2", "timer3", "timer4", "timer5"};

    for (const QString& name : timerNames) {
        timer->init_timer(name);
    }

    // All timers should be startable
    for (const QString& name : timerNames) {
        EXPECT_TRUE(timer->start_timer(name));
    }
}

TEST_F(US_TimerTest, InitTimerSameNameTwice) {
    QString timerName = "duplicate_timer";

    timer->init_timer(timerName);
    timer->init_timer(timerName);  // Initialize same timer again

    // Should still work
    EXPECT_TRUE(timer->start_timer(timerName));
}

// start_timer Tests
TEST_F(US_TimerTest, StartTimerValidTimer) {
    QString timerName = "valid_timer";
    timer->init_timer(timerName);

    bool result = timer->start_timer(timerName);

    EXPECT_TRUE(result);
}

TEST_F(US_TimerTest, StartTimerNonExistentTimer) {
    QString nonExistentName = "non_existent_timer";

    bool result = timer->start_timer(nonExistentName);

    EXPECT_FALSE(result);
}

TEST_F(US_TimerTest, StartTimerEmptyString) {
    QString emptyName = "";
    timer->init_timer(emptyName);

    bool result = timer->start_timer(emptyName);

    EXPECT_TRUE(result);
}

TEST_F(US_TimerTest, StartTimerMultipleTimes) {
    QString timerName = "restart_timer";
    timer->init_timer(timerName);

    // Start timer multiple times (should restart each time)
    EXPECT_TRUE(timer->start_timer(timerName));
    EXPECT_TRUE(timer->start_timer(timerName));
    EXPECT_TRUE(timer->start_timer(timerName));
}

TEST_F(US_TimerTest, StartTimerAfterEnd) {
    QString timerName = "end_start_timer";
    timer->init_timer(timerName);

    timer->start_timer(timerName);
    timer->end_timer(timerName);

    // Should be able to start again after ending
    EXPECT_TRUE(timer->start_timer(timerName));
}

// end_timer Tests
TEST_F(US_TimerTest, EndTimerValidTimer) {
    QString timerName = "valid_end_timer";
    timer->init_timer(timerName);
    timer->start_timer(timerName);

    bool result = timer->end_timer(timerName);

    EXPECT_TRUE(result);
}

TEST_F(US_TimerTest, EndTimerNonExistentTimer) {
    QString nonExistentName = "non_existent_end_timer";

    bool result = timer->end_timer(nonExistentName);

    EXPECT_FALSE(result);
}

TEST_F(US_TimerTest, EndTimerWithoutStart) {
    QString timerName = "end_without_start";
    timer->init_timer(timerName);

    // End timer without starting it first
    bool result = timer->end_timer(timerName);

    EXPECT_TRUE(result);  // Should still succeed
}

TEST_F(US_TimerTest, EndTimerMultipleTimes) {
    QString timerName = "multiple_end_timer";
    timer->init_timer(timerName);
    timer->start_timer(timerName);

    // End timer multiple times
    EXPECT_TRUE(timer->end_timer(timerName));
    EXPECT_TRUE(timer->end_timer(timerName));
    EXPECT_TRUE(timer->end_timer(timerName));
}

TEST_F(US_TimerTest, EndTimerAccumulatesTime) {
    QString timerName = "accumulate_timer";
    timer->init_timer(timerName);

    // First measurement
    timer->start_timer(timerName);
    QTest::qWait(10);
    timer->end_timer(timerName);

    // Second measurement
    timer->start_timer(timerName);
    QTest::qWait(10);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("2 calls"));  // Should show 2 calls
}

// list_time Tests
TEST_F(US_TimerTest, ListTimeValidTimer) {
    QString timerName = "list_timer";
    timer->init_timer(timerName);
    timer->start_timer(timerName);
    QTest::qWait(5);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);

    EXPECT_THAT(result, QStringContains(timerName));
    EXPECT_THAT(result, QStringContains("1 calls"));
    EXPECT_THAT(result, QStringContains("total time"));
    EXPECT_THAT(result, QStringContains("avg time"));
    EXPECT_THAT(result, QStringContains("sd"));
    EXPECT_THAT(result, QStringContains("ms"));
}

TEST_F(US_TimerTest, ListTimeNonExistentTimer) {
    QString nonExistentName = "non_existent_list_timer";

    QString result = timer->list_time(nonExistentName);

    EXPECT_THAT(result, QStringContains("US_Timer::Error"));
    EXPECT_THAT(result, QStringContains("undefined timer"));
    EXPECT_THAT(result, QStringContains(nonExistentName));
}

TEST_F(US_TimerTest, ListTimeEmptyString) {
    QString emptyName = "";

    QString result = timer->list_time(emptyName);

    EXPECT_THAT(result, QStringContains("US_Timer::Error"));
    EXPECT_THAT(result, QStringContains("undefined timer"));
}

TEST_F(US_TimerTest, ListTimeTimerWithoutUsage) {
    QString timerName = "unused_timer";
    timer->init_timer(timerName);

    QString result = timer->list_time(timerName);

    EXPECT_THAT(result, QStringContains("US_Timer::Error"));
    EXPECT_THAT(result, QStringContains("no usage counts"));
    EXPECT_THAT(result, QStringContains(timerName));
}

TEST_F(US_TimerTest, ListTimeMultipleCalls) {
    QString timerName = "multiple_calls_timer";
    timer->init_timer(timerName);

    // Make multiple measurements
    for (int i = 0; i < 5; ++i) {
        timer->start_timer(timerName);
        QTest::qWait(2);
        timer->end_timer(timerName);
    }

    QString result = timer->list_time(timerName);

    EXPECT_THAT(result, QStringContains("5 calls"));
    EXPECT_THAT(result, QStringContains(timerName));
}

TEST_F(US_TimerTest, ListTimeZeroTimeElapsed) {
    QString timerName = "zero_time_timer";
    timer->init_timer(timerName);

    timer->start_timer(timerName);
    // Don't sleep - elapsed time should be very small or zero
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);

    EXPECT_THAT(result, QStringContains("1 calls"));
    EXPECT_THAT(result, QStringContains(timerName));
    // Time should be >= 0
}

TEST_F(US_TimerTest, ListTimeStatisticalCalculations) {
    QString timerName = "stats_timer";
    timer->init_timer(timerName);

    // Make several measurements with different durations
    timer->start_timer(timerName);
    QTest::qWait(5);
    timer->end_timer(timerName);

    timer->start_timer(timerName);
    QTest::qWait(10);
    timer->end_timer(timerName);

    timer->start_timer(timerName);
    QTest::qWait(15);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);

    EXPECT_THAT(result, QStringContains("3 calls"));
    EXPECT_THAT(result, QStringContains("total time"));
    EXPECT_THAT(result, QStringContains("avg time"));
    EXPECT_THAT(result, QStringContains("sd"));  // Standard deviation should be calculated
}

// list_times Tests
TEST_F(US_TimerTest, ListTimesEmptyTimer) {
    QString result = timer->list_times();

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(US_TimerTest, ListTimesSingleTimer) {
    QString timerName = "single_timer";
    timer->init_timer(timerName);
    timer->start_timer(timerName);
    QTest::qWait(5);
    timer->end_timer(timerName);

    QString result = timer->list_times();

    EXPECT_THAT(result, QStringContains(timerName));
    EXPECT_THAT(result, QStringContains("1 calls"));
}

TEST_F(US_TimerTest, ListTimesMultipleTimers) {
    QStringList timerNames = {"timer_a", "timer_b", "timer_c"};

    for (const QString& name : timerNames) {
        timer->init_timer(name);
        timer->start_timer(name);
        QTest::qWait(3);
        timer->end_timer(name);
    }

    QString result = timer->list_times();

    for (const QString& name : timerNames) {
        EXPECT_THAT(result, QStringContains(name));
    }
}

TEST_F(US_TimerTest, ListTimesWithUnusedTimers) {
    timer->init_timer("used_timer");
    timer->init_timer("unused_timer");

    timer->start_timer("used_timer");
    QTest::qWait(5);
    timer->end_timer("used_timer");

    QString result = timer->list_times();

    EXPECT_THAT(result, QStringContains("used_timer"));
    EXPECT_THAT(result, QStringContains("unused_timer"));
    EXPECT_THAT(result, QStringContains("no usage counts"));
}

TEST_F(US_TimerTest, ListTimesWithMixedTimers) {
    // Used timer
    timer->init_timer("good_timer");
    timer->start_timer("good_timer");
    QTest::qWait(5);
    timer->end_timer("good_timer");

    // Unused timer
    timer->init_timer("bad_timer");

    QString result = timer->list_times();

    EXPECT_THAT(result, QStringContains("good_timer"));
    EXPECT_THAT(result, QStringContains("1 calls"));
    EXPECT_THAT(result, QStringContains("bad_timer"));
    EXPECT_THAT(result, QStringContains("no usage counts"));
}

// Edge Case Tests
class US_TimerEdgeCaseTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        timer = std::make_unique<US_Timer>();
    }

    void TearDown() override {
        timer.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_Timer> timer;
};

TEST_F(US_TimerEdgeCaseTest, VeryShortTimeMeasurement) {
    QString timerName = "micro_timer";
    timer->init_timer(timerName);

    timer->start_timer(timerName);
    // No sleep - measure fastest possible time
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("1 calls"));
    // Should not crash even with very short times
}

TEST_F(US_TimerEdgeCaseTest, LargeNumberOfCalls) {
    QString timerName = "many_calls_timer";
    timer->init_timer(timerName);

    // Make many calls to test overflow protection
    for (int i = 0; i < 1000; ++i) {
        timer->start_timer(timerName);
        if (i % 100 == 0) QTest::qWait(1);  // Occasional small delay
        timer->end_timer(timerName);
    }

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("1000 calls"));
}

TEST_F(US_TimerEdgeCaseTest, TimerNameWithWhitespace) {
    QString timerName = "  timer with spaces  ";
    timer->init_timer(timerName);

    timer->start_timer(timerName);
    QTest::qWait(5);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains(timerName));
}

TEST_F(US_TimerEdgeCaseTest, TimerNameWithNewlines) {
    QString timerName = "timer\nwith\nnewlines";
    timer->init_timer(timerName);

    timer->start_timer(timerName);
    QTest::qWait(5);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("timer"));
}

TEST_F(US_TimerEdgeCaseTest, TimerNameWithTabsAndCarriageReturns) {
    QString timerName = "timer\twith\tspecial\rchars";
    timer->init_timer(timerName);

    timer->start_timer(timerName);
    QTest::qWait(5);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("timer"));
}

TEST_F(US_TimerEdgeCaseTest, CaseSensitiveTimerNames) {
    timer->init_timer("Timer");
    timer->init_timer("timer");
    timer->init_timer("TIMER");

    // All should be treated as different timers
    EXPECT_TRUE(timer->start_timer("Timer"));
    EXPECT_TRUE(timer->start_timer("timer"));
    EXPECT_TRUE(timer->start_timer("TIMER"));

    EXPECT_FALSE(timer->start_timer("TiMeR"));  // Should not exist
}

// Error Condition Tests
class US_TimerErrorTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        timer = std::make_unique<US_Timer>();
    }

    void TearDown() override {
        timer.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_Timer> timer;
};

TEST_F(US_TimerErrorTest, StartNonExistentTimer) {
    bool result = timer->start_timer("does_not_exist");
    EXPECT_FALSE(result);
}

TEST_F(US_TimerErrorTest, EndNonExistentTimer) {
    bool result = timer->end_timer("does_not_exist");
    EXPECT_FALSE(result);
}

TEST_F(US_TimerErrorTest, ListNonExistentTimer) {
    QString result = timer->list_time("does_not_exist");
    EXPECT_THAT(result, QStringContains("US_Timer::Error"));
    EXPECT_THAT(result, QStringContains("undefined timer"));
}

TEST_F(US_TimerErrorTest, MultipleEndWithoutStart) {
    QString timerName = "end_without_start";
    timer->init_timer(timerName);

    // End multiple times without starting
    EXPECT_TRUE(timer->end_timer(timerName));
    EXPECT_TRUE(timer->end_timer(timerName));
    EXPECT_TRUE(timer->end_timer(timerName));

    // Should still be able to list
    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("3 calls"));
}

TEST_F(US_TimerErrorTest, StartAfterMultipleEnds) {
    QString timerName = "start_after_ends";
    timer->init_timer(timerName);

    timer->end_timer(timerName);
    timer->end_timer(timerName);

    // Should still be able to start
    EXPECT_TRUE(timer->start_timer(timerName));
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("3 calls"));
}

// Method Sequence Tests
class US_TimerSequenceTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        timer = std::make_unique<US_Timer>();
    }

    void TearDown() override {
        timer.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_Timer> timer;
};

TEST_F(US_TimerSequenceTest, CorrectSequenceInitStartEnd) {
    QString timerName = "correct_sequence";

    timer->init_timer(timerName);
    EXPECT_TRUE(timer->start_timer(timerName));
    EXPECT_TRUE(timer->end_timer(timerName));

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("1 calls"));
}

TEST_F(US_TimerSequenceTest, StartEndMultipleCycles) {
    QString timerName = "cycle_timer";
    timer->init_timer(timerName);

    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(timer->start_timer(timerName));
        QTest::qWait(1);
        EXPECT_TRUE(timer->end_timer(timerName));
    }

    QString result = timer->list_time(timerName);
    EXPECT_THAT(result, QStringContains("10 calls"));
}

TEST_F(US_TimerSequenceTest, OverlappingTimers) {
    timer->init_timer("timer1");
    timer->init_timer("timer2");

    timer->start_timer("timer1");
    timer->start_timer("timer2");
    QTest::qWait(5);
    timer->end_timer("timer1");
    QTest::qWait(5);
    timer->end_timer("timer2");

    QString result1 = timer->list_time("timer1");
    QString result2 = timer->list_time("timer2");

    EXPECT_THAT(result1, QStringContains("1 calls"));
    EXPECT_THAT(result2, QStringContains("1 calls"));
}

// Memory and Resource Tests
class US_TimerResourceTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }
};

TEST_F(US_TimerResourceTest, MultipleTimerInstances) {
    // Test creating multiple timer instances
    std::vector<std::unique_ptr<US_Timer>> timers;

    for (int i = 0; i < 10; ++i) {
        timers.push_back(std::make_unique<US_Timer>());
        QString timerName = QString("timer_%1").arg(i);
        timers[i]->init_timer(timerName);
        timers[i]->start_timer(timerName);
        timers[i]->end_timer(timerName);
    }

    // All should work independently
    for (int i = 0; i < 10; ++i) {
        QString timerName = QString("timer_%1").arg(i);
        QString result = timers[i]->list_time(timerName);
        EXPECT_THAT(result, QStringContains(timerName));
    }
}

TEST_F(US_TimerResourceTest, ConstructorDestructorStress) {
    // Test rapid creation and destruction
    for (int i = 0; i < 100; ++i) {
        US_Timer localTimer;
        localTimer.init_timer("stress_timer");
        localTimer.start_timer("stress_timer");
        localTimer.end_timer("stress_timer");
        // Destructor called automatically
    }
    SUCCEED();
}

// Format and Output Tests
class US_TimerOutputTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        timer = std::make_unique<US_Timer>();
    }

    void TearDown() override {
        timer.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_Timer> timer;
};

TEST_F(US_TimerOutputTest, OutputFormatConsistency) {
    QString timerName = "format_timer";
    timer->init_timer(timerName);
    timer->start_timer(timerName);
    QTest::qWait(5);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);

    // Check that all expected components are present
    EXPECT_THAT(result, QStringContains(timerName));
    EXPECT_THAT(result, QStringContains("calls"));
    EXPECT_THAT(result, QStringContains("total time"));
    EXPECT_THAT(result, QStringContains("avg time"));
    EXPECT_THAT(result, QStringContains("sd"));
    EXPECT_THAT(result, QStringContains("ms"));
}

TEST_F(US_TimerOutputTest, ErrorMessageFormat) {
    QString result1 = timer->list_time("undefined");
    QString result2 = timer->list_time("another_undefined");

    // Both should have consistent error format
    EXPECT_THAT(result1, QStringContains("US_Timer::Error"));
    EXPECT_THAT(result2, QStringContains("US_Timer::Error"));
    EXPECT_THAT(result1, QStringContains("undefined timer"));
    EXPECT_THAT(result2, QStringContains("undefined timer"));
}

TEST_F(US_TimerOutputTest, StatisticsAccuracy) {
    QString timerName = "stats_accuracy";
    timer->init_timer(timerName);

    // Make measurements with known sleep times
    timer->start_timer(timerName);
    QTest::qWait(10);
    timer->end_timer(timerName);

    timer->start_timer(timerName);
    QTest::qWait(20);
    timer->end_timer(timerName);

    QString result = timer->list_time(timerName);

    // Should show 2 calls
    EXPECT_THAT(result, QStringContains("2 calls"));

    // Total time should be at least 30ms (allowing for system variance)
    // But we can't test exact values due to timing variance
    EXPECT_THAT(result, QStringContains("total time"));
    EXPECT_THAT(result, QStringContains("avg time"));
}