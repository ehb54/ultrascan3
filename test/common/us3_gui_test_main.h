#pragma once

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include "us_defines.h"
#include "us_settings.h"

// Native QTest normally creates QApplication through QTEST_MAIN.  UltraScan
// tests need to configure the platform plugin and isolate writable settings
// first, so UI tests use this equivalent main instead.
#define US3_GUI_TEST_MAIN(TestClass)                                           \
int main(int argc, char** argv)                                                \
{                                                                              \
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))                        \
        qputenv("QT_QPA_PLATFORM", "offscreen");                              \
    if (qEnvironmentVariableIsEmpty("QT_OPENGL"))                              \
        qputenv("QT_OPENGL", "software");                                    \
    if (qEnvironmentVariableIsEmpty("LIBGL_ALWAYS_SOFTWARE"))                  \
        qputenv("LIBGL_ALWAYS_SOFTWARE", "1");                               \
                                                                               \
    QStandardPaths::setTestModeEnabled(true);                                  \
    QApplication app(argc, argv);                                              \
    QCoreApplication::setApplicationName("ultrascan3-qtest");                 \
    QCoreApplication::setOrganizationName(US3);                                \
                                                                               \
    QTemporaryDir sandbox(QDir::tempPath() +                                   \
                          "/ultrascan3-gui-test-XXXXXX");                      \
    if (!sandbox.isValid())                                                     \
        return 2;                                                              \
                                                                               \
    const QString settingsRoot = sandbox.path() + "/settings";                \
    const QString workRoot = sandbox.path() + "/work";                        \
    const QString importRoot = workRoot + "/imports";                         \
    const QString temporaryRoot = workRoot + "/tmp";                          \
    QDir().mkpath(settingsRoot);                                                \
    QDir().mkpath(importRoot);                                                  \
    QDir().mkpath(temporaryRoot);                                               \
                                                                               \
    qputenv("US3_TEST_SANDBOX", QFile::encodeName(sandbox.path()));            \
    qputenv("US3_TEST_SETTINGS_ROOT", QFile::encodeName(settingsRoot));        \
    qputenv("US3_TEST_WORK_ROOT", QFile::encodeName(workRoot));                \
    /* setPath() does not redirect NativeFormat on macOS (CFPreferences) or  */\
    /* Windows (registry), so make IniFormat the default to keep every       */\
    /* QSettings built without an explicit format inside the sandbox.        */\
    QSettings::setDefaultFormat(QSettings::IniFormat);                         \
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,           \
                       settingsRoot);                                           \
    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope,         \
                       settingsRoot);                                           \
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,              \
                       settingsRoot);                                           \
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope,            \
                       settingsRoot);                                           \
    US_Settings::set_workBaseDir(workRoot);                                     \
    US_Settings::set_importDir(importRoot);                                     \
    US_Settings::set_tmpDir(temporaryRoot);                                     \
                                                                               \
    TestClass testObject;                                                       \
    return QTest::qExec(&testObject, argc, argv);                               \
}
