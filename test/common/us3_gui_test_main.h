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
                                                                               \
    /* Everything that redirects the settings store has to happen before     */\
    /* QApplication is constructed.  Building the application reads settings */\
    /* through the theme, and US_SettingsStore resolves where its store lives*/\
    /* on the first read and then keeps it, so a redirect set afterwards     */\
    /* arrives too late and the process writes to the real preferences.      */\
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
    /* setPath alone isolates Linux only: the native store is CFPreferences  */\
    /* or the registry on macOS and Windows, where setPath has no effect, so */\
    /* US_SettingsStore has to be told to use a file instead.                */\
    qputenv("US3_SETTINGS_ROOT", QFile::encodeName(settingsRoot));             \
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,              \
                       settingsRoot);                                           \
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope,            \
                       settingsRoot);                                           \
                                                                               \
    QApplication app(argc, argv);                                              \
    QCoreApplication::setApplicationName("ultrascan3-qtest");                 \
    QCoreApplication::setOrganizationName(US3);                                \
                                                                               \
    US_Settings::set_workBaseDir(workRoot);                                     \
    US_Settings::set_importDir(importRoot);                                     \
    US_Settings::set_tmpDir(temporaryRoot);                                     \
                                                                               \
    TestClass testObject;                                                       \
    return QTest::qExec(&testObject, argc, argv);                               \
}
