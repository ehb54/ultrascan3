// test/common/qt_test_base.h
#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>          // Add this for MATCHER_P
#include <QCoreApplication>
#include <QApplication>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <memory>

/**
 * Base class for all Qt-related tests
 */
class QtTestBase : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!QCoreApplication::instance()) {
            static int argc = 1;
            static char* argv[] = {const_cast<char*>("test")};
            app_ = std::make_unique<QCoreApplication>(argc, argv);
        }
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    void SetUp() override {}
    void TearDown() override {}

    // Qt utilities
    void qtWait(int ms) { QTest::qWait(ms); }

    bool waitForSignal(QObject* sender, const char* signal, int timeoutMs = 5000) {
        QSignalSpy spy(sender, signal);
        return spy.wait(timeoutMs);
    }

    void processEvents() { QCoreApplication::processEvents(); }

private:
    static std::unique_ptr<QCoreApplication> app_;
};

/**
 * Base class for GUI tests that need QApplication
 */
class GuiTestBase : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!QApplication::instance()) {
            static int argc = 1;
            static char* argv[] = {const_cast<char*>("test")};
            app_ = std::make_unique<QApplication>(argc, argv);
        }
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    void SetUp() override {}
    void TearDown() override {}

    // GUI-specific utilities
    void clickButton(const QString& objectName) {
        auto* widget = QApplication::activeWindow();
        if (auto* button = widget->findChild<QPushButton*>(objectName)) {
            QTest::mouseClick(button, Qt::LeftButton);
        }
    }

    void enterText(const QString& objectName, const QString& text) {
        auto* widget = QApplication::activeWindow();
        if (auto* lineEdit = widget->findChild<QLineEdit*>(objectName)) {
            lineEdit->clear();
            QTest::keyClicks(lineEdit, text);
        }
    }

private:
    static std::unique_ptr<QApplication> app_;
};

/**
 * Custom Google Test matchers for Qt types
 */
namespace qt_matchers {

    MATCHER_P(QStringEq, expected, "") {
        return arg == expected;
    }

    MATCHER_P(QStringContains, substring, "") {
        return arg.contains(substring);
    }

    MATCHER(QStringIsEmpty, "") {
        return arg.isEmpty();
    }

    MATCHER_P(SignalEmittedTimes, count, "") {
        return arg.count() == count;
    }

} // namespace qt_matchers

/**
 * Convenience macros for common Qt test patterns
 */
#define EXPECT_SIGNAL_EMITTED(object, signal, action) \
    do { \
        QSignalSpy spy(object, signal); \
        action; \
        EXPECT_GT(spy.count(), 0) << "Expected signal " #signal " to be emitted"; \
    } while(0)

#define EXPECT_SIGNAL_COUNT(object, signal, expected_count, action) \
    do { \
        QSignalSpy spy(object, signal); \
        action; \
        EXPECT_EQ(spy.count(), expected_count); \
    } while(0)

#define EXPECT_NO_SIGNAL(object, signal, action) \
    do { \
        QSignalSpy spy(object, signal); \
        action; \
        EXPECT_EQ(spy.count(), 0) << "Expected no " #signal " signal"; \
    } while(0)
